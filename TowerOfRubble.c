#include "TowerOfRubble.h"

u8 HighScoreCheck(u8 p){
	return 0;
}



void HighScoreSave(u8 p){

}



u8 IsSolidT(u8 x, u8 y){
	u8 t = vram[(y*VRAM_TILES_H)+x]-RAM_TILES_COUNT;
	if(t < FIRST_GROUND_TILE || t > LAST_GROUND_TILE)
		return 0;
	return 1;
}



u8 IsSolidP(u8 x, u8 y){
	x>>=3;
	y>>=3;
	u8 t = vram[(y*VRAM_TILES_H)+x]-RAM_TILES_COUNT;
	if(t < FIRST_GROUND_TILE || t > LAST_GROUND_TILE)
		return 0;
	return 1;
}



void ResetSprites(){
	sprite_num = 0;
	for(u8 i=0;i<MAX_SPRITES;i++)
		sprites[i].flags = SPRITE_OFF;
}



void SetSprite(u8 x, u8 y, u8 t, u8 f){
	sprites[sprite_num].x = x;
	sprites[sprite_num].y = y;
	sprites[sprite_num].tileIndex = t;
	sprites[sprite_num++].flags = f;
}



u8 UpdatePlayers(){
	ResetSprites();
	u8 score_tick = 0;
	if(game_state & GS_TITLE_SCREEN)
		TitleScreenUpdate();
	else if(global_frame60 == 59)
		score_tick = 1;

	u8 num_alive = 0;

	for(u8 p=0; p<=pmax; p++){
		last_pad[p] = (p?joypad2_status_lo:joypad1_status_lo);
		pad[p] = ReadJoypad(p);

		if(pstate[p] == STATE_DEAD)
			continue;

		if(score_tick)
			pscore[p]++;
//TriggerFx(1,255,1);
		u8 cx = px[p]>>3;
		u8 cy = py[p]>>3;

		//if(py > (SCREEN_TILES_V-1)*8){//lava death
		//	pstate[p] = STATE_DYING;
		//}

		if(pstate[p] == STATE_DYING){
			if(pframe[p] > 5){//done animating explosion?
				px[p] = py[p] = 0;//used for score text entry
				pstate[p] = STATE_HISCORE;
			}else{
				pframe[p]++;
				continue;
			}
		}

		if(pstate[p] == STATE_HISCORE){
			if(!HighScoreCheck(p)){//possible both players competing for the last position...
				pstate[p] = STATE_DEAD;
			}else{
				if((pad[p] & BTN_START) && !(last_pad[p] & BTN_START)){
					pstate[p] = STATE_DEAD;
					HighScoreSave(p);
				}else if(px[p] && (pad[p] & BTN_LEFT) && !(last_pad[p] & BTN_LEFT)){
					px[p]--;
				}else if(px[p] < 8 && (pad[p] & BTN_RIGHT) && !(last_pad[p] & BTN_RIGHT)){
					px[p]++;
				}else if((pad[p] & BTN_UP) && !(last_pad[p] & BTN_UP)){
					u16 voff = vram[5+(p*15)+px[p]];
					if(++vram[voff] > 'Z')
						vram[voff] = 'A';
				}else if((pad[p] & BTN_DOWN) && !(last_pad[p] & BTN_DOWN)){
					u16 voff = vram[5+(p*15)+px[p]];
					if(--vram[voff] < 'A')
						vram[voff] = 'Z';
				}
			}
			if(pstate[p] == STATE_DEAD){//clean up score entry

			}
			continue;
		}
		num_alive++;

		if(pstate[p] == STATE_IDLE){//outcomes: (change)RIGHT, FALL, RUN, HOP, CLIMB_U, CLIMB_D
			pframe[p] = 32;
			if(IsSolidT(cx,cy+1) == 0){
//TriggerFx(1,255,1);
				pstate[p] |= STATE_FALLING;
				pframe[p] = 14;
				poffset[p] = 0;
			}
			if(pftime[p]){//turning?
				if(--pftime[p] < 1){
					if(pstate[p] & STATE_RIGHT)
						pstate[p] ^= STATE_RIGHT;
					else
						pstate[p] |= STATE_RIGHT;
				}else
					continue;
			}
			if(((pstate[p] & STATE_RIGHT) && (pad[p] & BTN_LEFT)) || (!(pstate[p] & STATE_RIGHT) && (pad[p] & BTN_RIGHT)))
				pftime[p] = 8;//initiate turning
			if((pad[p] & BTN_DOWN) && !(last_pad[p] & BTN_DOWN) && !IsSolidT(cx+((pstate[p]&STATE_RIGHT)?-1:1),cy+1)){
				pstate[p] |= STATE_CLIMB_D;
				pframe[p] = 26;
				poffset[p] = 1;
			}else if(!pftime[p]){
				if((pad[p] & BTN_RIGHT)){
					px[p]++;
				}else if((pad[p] & BTN_LEFT)){
					px[p]--;
				}
			}
		}

		if(pstate[p] & STATE_FALLING){//outcomes: (change)RIGHT, HOP(wall jump), IDLE
			pframe[p] = 14;
			py[p]++;
			if(poffset[p] < 8)
				poffset[p]++;
			if(poffset[p] == 8){
				poffset[p] = 0;
				//if(!IsSolidT(cx+((pstate[p] & STATE_RIGHT)?1:-1),cy)){//ledge to grab?
				//	pstate[p] ^= STATE_FALLING;
				//	pstate[p] |= STATE_HANG;
				if(IsSolidT(cx,cy+2)){//}else if(IsSolidT(cx,cy+2)){//hit ground?
					pstate[p] ^= STATE_FALLING;//STATE_IDLE
		//TriggerFx(2,255,1);
				}
				//pframe[p] = 0+(poffset[p] < 3?poffset[p]:3);
			}//else
			//	pframe[p] = poffset[p];

		}else if(pstate[p] & STATE_RUN){
			if(poffset[p] < 8){
				if(((pstate[p] & STATE_RIGHT) && (pad[p] & BTN_LEFT)) || (!(pstate[p] & STATE_RIGHT) && (pad[p] & BTN_RIGHT))){//change direction?
					if(pstate[p] & STATE_RIGHT)
						pstate[p] ^= STATE_RIGHT;
					else
						pstate[p] |= STATE_RIGHT;
					poffset[p] = 8-poffset[p];
				}else
					poffset[p]++;
				pframe[p] = 36+(poffset[p]/2);
			}else{
				px[p] += ((pstate[p] & STATE_RIGHT)?1:-1);
				poffset[p] = 0;
				if(((pstate[p] & STATE_RIGHT) && (pad[p] & BTN_RIGHT)) || (!(pstate[p] & STATE_RIGHT) && (pad[p] & BTN_LEFT))){
					continue;
				}else{
					pstate[p] ^= STATE_RUN;//IDLE
					pframe[p] = 0;
				}
			}

			if(IsSolidT(cx+((pstate[p] & STATE_RIGHT)?1:-1),cy)){//destructive beams will spend 1 frame as solid...
				pstate[p] = STATE_DYING;
				pframe[p] = 0;
			}
			
		}else if(pstate[p] & STATE_CLIMB_U){//outcomes: CLIMB_D, IDLE(on top)
			if(poffset[p] < 8){
				if(pad[p] & BTN_DOWN){

					pstate[p] ^= STATE_CLIMB_U;
					poffset[p] = 8-poffset[p];
					continue;
				}else
					poffset[p]++;
			}
			if(IsSolidT(cx+1,cy-1)){//a new block fell so we can't climb?
				if(poffset[p] < 5){//no squashed, but must fall
					pstate[p] ^= STATE_CLIMB_U;
					pstate[p] |= STATE_FALLING;
					poffset[p] = 0;
					pframe[p] = 0;
				}
			}
		}else if(pstate[p] & STATE_CLIMB_D){//outcomes: CLIMB_U, IDLE(on bottom)
			
		}
		num_alive++;
	}

	for(u8 p=0; p<=pmax; p++){
		if(pstate[p] == STATE_DEAD)
			continue;
		SetSprite(px[p],py[p],pframe[p]+(p*48),0);
	}
	return num_alive;
}


u32 SpiRamCopyByIndex(u32 dst, u8 src, char *buf){
	u16 soff = SpiRamReadU32(0, src*4);//get the SD offset to data(as sector counts due to padding option in dconvert)
	u16 send = SpiRamReadU32(0, (src+1)*4);//get the next directory entry to determine the length
	u16 num_sectors = (send-soff)/512UL;

	FS_Set_Pos(&sd_struct, (u32)(soff*512UL));
	while(num_sectors--){
		FS_Read_Sector(&sd_struct);
		FS_Next_Sector(&sd_struct);
		src += 512UL;
		SpiRamSeqWriteStart((dst>65535UL), dst&0xFFFF);
		for(u16 i=0;i<512;i++)
			SpiRamSeqWriteU8(buf[i]);
		SpiRamSeqWriteEnd();
		dst += 512UL;
	}
	return dst;
}

void LoadMap(const u8 *map){
	WaitVsync(1);
	global_frame = global_frame60 = 0;
	beam_v_next = 0;//single block every second
	beam_v_wide_next = 23*60;//2 horizontal blocks(starting at 23, then 33,43,53,etc.)
	beam_v_tall_next[0] = 8*60;//2 vertical blocks(these will sometimes sync together..)
	beam_v_tall_next[1] = 18*60;
	beam_v_tall_next[2] = 63*80;//?
	beam_h_next = 30*60;//initial starts at 30, then at 45,60,75,etc.
	island_dissolve_next = 11*60;//dissolves 2 sections of 2 horizontal tiles(on the same island, starts at 11, then 21, 31, etc);

	if(!(game_state & GS_TITLE_SCREEN))
		ClearVram();
	PopulateStars();

	u8 h = pgm_read_byte(map++);
	for(u8 i=0;i<MAX_PLAYERS;i++){
		if(i > pmax){
			map += 2;
			continue;
		}
		px[i] = (pgm_read_byte(map++))*8;
		py[i] = ((SCREEN_TILES_V-1)-pgm_read_byte(map++))*8;
		pframe[i] = 32;
		if(px[i] < (SCREEN_TILES_H/2)*8)
			pstate[i] = STATE_IDLE|STATE_RIGHT;
		else
			pstate[i] = STATE_IDLE;
	}
	u8 bits = 0;
	u8 t;
	for(u8 y=SCREEN_TILES_V-h;y<SCREEN_TILES_V;y++){
		bits = 0;
		for(u8 x=0;x<SCREEN_TILES_H;x++){
			if(!bits){
				t = pgm_read_byte(map++);
				bits = 8;
			}
			if(t&128){
				u8 g = GetPrngNumber(0)%NUM_GROUND_TILES;
				g *= (NUM_DISSOLVE_FRAMES+1);
				SetTile(x,y,FIRST_GROUND_TILE+g);
				//u8 g = ((y*x)*bits)%NUM_GROUND_TILES;
				//g *= (NUM_DISSOLVE_FRAMES+g);
				//SetTile(x,y,FIRST_GROUND_TILE+g);
			}else if(y == SCREEN_TILES_V-1)//set initial lava tiles so animation works right
				SetTile(x,y,FIRST_LAVA_TILE+1);
			t<<=1;
			bits--;
		}
	}
}



void LoadSpiRamSong(u8 s){//TODO make this re-entrant
	if(game_state & (GS_NO_SD|GS_NO_SPIR))
		return;

	u32 soff = SpiRamReadU32(0, s*4);//get the full SD offset to data for this song
	u32 send = SpiRamReadU32(0, (s+1)*4);
	u16 num_sectors = send-soff;
	u32 spoff = 512;//location to write first song byte to SPI RAM
/*
	FS_Set_Pos(&sd_struct, soff);
	while(num_sectors--){//load entire song
		FS_Read_Sector(&sd_struct);
		FS_Next_Sector(&sd_struct);
		SpiRamSeqWriteStart((spoff>65535UL), spoff&0xFFFF);
		for(u16 i=0;i<512;i++)
			SpiRamSeqWriteU8(ram_tiles[i]);
		SpiRamSeqWriteEnd();
		spoff += 512UL;
	}
*/
	game_state |= GS_SONG_LOADED;
}

const char ub_string[] PROGMEM = {'U','Z','E','B','O','X'};
void Intro(){
//ClearVram();
//for(u16 i=0;i<VRAM_SIZE;i++)
//	vram[i] = RAM_TILES_COUNT+FIRST_GROUND_TILE;
//while(1);
//return;

	ResetSprites();
	ClearVram();
	FadeOut(0,1);
	PopulateStars();
	FadeIn(4,0);

	for(u8 i=0;i<6;i++){//copy over text data to ram_tiles(so we can recolor a bit)
		char c = pgm_read_byte(&ub_string[i])-28;
		for(u8 j=0;j<64;j++){
			u8 t = pgm_read_byte(&tile_data[(c*64)+j]);
			if(t)
				ram_tiles[(i*64)+j] = t&0b00000111;
		}
	}
	u8 x_start = ((SCREEN_TILES_H/2)-(6/2));
	for(u8 i=0;i<6;i++){//each letter
		for(u8 y=0;y<12;y++){//each falling position
			for(u8 j=0;j<y;j++)//draw beam above
				vram[x_start+i+(j*VRAM_TILES_H)] = RAM_TILES_COUNT+27+((y>>1)%2);
			vram[x_start+i+(y*VRAM_TILES_H)] = i;//point to ram_tile[] version of font tiles
			for(u8 j=0;j<2;j++){
				AnimateStars();
				WaitVsync(1);
			}
		}
		RestoreStarColumn(x_start+i);
		vram[x_start+i+(x_start*VRAM_TILES_H)] = i;
		//TriggerFx(0,255,1);
	}
	for(u8 i=0;i<120;i++){
		AnimateStars();
		WaitVsync(1);
		for(u8 j=0;j<2+(i/10);j++)
			ram_tiles[GetPrngNumber(0)%(64*6)] = 0;
		if(i == 45)
			FadeOut(8,0);
	}
	ClearVram();
	FadeIn(4,0);
	TorPrint(3,1,PSTR("CREDITS"));
	TorPrint(3,4,PSTR("ORIGINAL VERSION `^_e"));
	TorPrint(6,5,PSTR("BY FLATGUB"));
	TorPrint(3,8,PSTR("Cdb PETSCII TITLE GFX"));
	TorPrint(6,9,PSTR("BY WARRIOR^"));
	TorPrint(3,12,PSTR("`d^^ PORT INSPIRATION"));
	TorPrint(6,13,PSTR("BY DIONOID"));
	TorPrint(3,16,PSTR("UZEBOX PORT `^`b"));
	TorPrint(6,17,PSTR("BY LEE WEBER"));
	TorPrint(3,22,PSTR("THANKS FOR THE INTEREST"));
	TorPrint(4,23,PSTR("UZEBOX FORUM MEMBERS]"));

	//setup SD while we wait
	ClearVsyncCounter();
	sd_struct.bufp = &(ram_tiles[0]);
	u8 res = FS_Init(&sd_struct);
	if(res != 0U){//mount failed
		game_state |= GS_NO_SD;
	}else{//mount worked
		if(!SpiRamInit()){
			game_state |= GS_NO_SPIR;
			ClearVram();
			TorPrint(5,10,PSTR("FAILED TO INITIALIZE"));
			TorPrint(5,11,PSTR("SPI RAM"));
			while(1);
		}else{//SPI RAM initialized
			//TorPrint(10,SCREEN_TILES_V-2,PSTR("SPI RAM ENHANCED"));
			u32 file_pos = FS_Find(&sd_struct,
				((u16)('T') << 8) | ((u16)('O')),
				((u16)('R') << 8) | ((u16)('_')),
				((u16)('D') << 8) | ((u16)('A') ),
				((u16)('T') << 8) | ((u16)('A') ),
				((u16)('B') << 8) | ((u16)('I') ),
				((u16)('N') << 8) | ((u16)(0)));

			if(file_pos == 0U){//failed to find resource file
				game_state |= GS_NO_SD;
				ClearVram();
				TorPrint(5,10,PSTR("FAILED TO LOAD SD"));
				TorPrint(5,11,PSTR("RESOURCE FILE"));
				while(1);
			}else{//found resource file
				FS_Select_Cluster(&sd_struct, file_pos);
				FS_Read_Sector(&sd_struct);//read the first sector which is the offset directory

				for(u8 i=0;i<4;i++){//get sector offsets to each item, then add the file base
					u8 soff = ram_tiles[(i*4)];//data is short enough to have < 256 sector offsets...
					u32 foff = file_pos+soff;//create raw offset for use by FS_Set_Pos()
					SpiRamWriteU32(0, (i*4), foff);//store for quick access later(no extra SD seek..)
				}
			}
		}
	}

	//u32 spiroff = (u32)((8UL*1024UL)+(i*512UL));
	u32 spiroff = (u32)(8UL*1024UL);
	for(u8 i=4+0;i<4+2;i++)//load map then tile data from SD to SPI RAM
		spiroff = SpiRamCopyByIndex(spiroff, i, ram_tiles);
	
	for(u8 f=0; f<7; f++){
		ClearVsyncCounter();
		u8 moff = (f*((10*2)+2));//skip width height(TODO skip this in dconvert instead)
		SpiRamSeqReadStart(0, moff);
		for(u8 y=0;y<2;y++){
			for(u8 x=0;x<10;x++){
				vram[(y*VRAM_TILES_H)+x] = SpiRamSeqReadU8();
			}
		}
		SpiRamSeqReadEnd();
		if(GetVsyncCounter() < 20)
			WaitVsync(20-GetVsyncCounter());
	}

	//load the title song while we wait for an initial smooth transition
	if(GetVsyncCounter() < 120){
		LoadSpiRamSong(0);
		if(GetVsyncCounter() < 120)
			WaitVsync(120-GetVsyncCounter());
	}else{//went past the mandatory 2 seconds wait, try to hide the loading in the optional part
		ClearVsyncCounter();
		LoadSpiRamSong(0);
		if(GetVsyncCounter() < 180){
			u8 wait = 180-GetVsyncCounter();

			for(u8 i=0;i<wait;i++){
				WaitVsync(1);
				if(ReadJoypad(0))
					break;
			}
		}
	}
	FadeOut(3,1);
	FadeIn(1,0);

	
}




void PopulateStars(){
	for(u16 i=0; i<VRAM_SIZE; i++){//this will draw the built-in moon map as well
		if(vram[i] > RAM_TILES_COUNT)//avoid any non-zero tile(ClearVram() isn't called if title screen is active)
			continue;
		vram[i] = pgm_read_byte(&star_map[i]);
	}
}


void AnimateStars(){
	u8 star_count = (VRAM_TILES_H/8);
	while(star_count--){//animate stars randomly
		u16 voff = (u16)(GetPrngNumber(0)%(VRAM_SIZE-VRAM_TILES_H));
		u8 t = vram[voff];
		if(t < RAM_TILES_COUNT+FIRST_STAR_TILE || t > RAM_TILES_COUNT+LAST_STAR_TILE)
			continue;
		if(t < RAM_TILES_COUNT+FIRST_STAR_TILE+(NUM_STAR_TILES/2))
			vram[voff] += (NUM_STAR_TILES/2);
		else
			vram[voff] -= (NUM_STAR_TILES/2);
	}
}

void AnimateLava(){
	lava_timer = (lava_timer+1)%(NUM_LAVA_TILES);

	u16 voff = VRAM_TILES_H*(SCREEN_TILES_V-1);
	for(u8 i=0;i<SCREEN_TILES_H;i++){
		u8 t = vram[voff];

		if(t < RAM_TILES_COUNT+FIRST_LAVA_TILE || t > RAM_TILES_COUNT+LAST_LAVA_TILE){//ground tiles, overlay with sprites
			SetSprite(i*8,(SCREEN_TILES_V-1)*8,FIRST_LAVA_SPRITE+lava_timer,0);
			voff++;
		}else{//no ground here, draw with tiles to save ram_tiles[]
			vram[voff++] = RAM_TILES_COUNT+FIRST_LAVA_TILE+lava_timer;
		}

	}
}

void CheckPlayerHit(u8 x, u8 y){
	for(u8 p=0;p<pmax;p++){
		if(pstate[p] >= STATE_DYING)
			continue;
		if(px[p]/8 == x && py[p]/8 == y){
			pstate[p] = STATE_DYING;
			TriggerFx(1,255,1);
		}
	}
}

void CheckPlayerHitColumn(u8 x){
	for(u8 p=0;p<pmax;p++){
		if(pstate[p] >= STATE_DYING)
			continue;
		if(px[p]/8 == x){
			pstate[p] = STATE_DYING;
			TriggerFx(1,255,1);
		}
	}
}

void CheckPlayerHitRow(u8 y){
	for(u8 p=0;p<pmax;p++){
		if(pstate[p] >= STATE_DYING)
			continue;
		if(py[p]/8 == y){
			pstate[p] = STATE_DYING;
			TriggerFx(1,255,1);
		}
	}
}

void RestoreStarColumn(u8 x){//this needs to be adjusted if SCREEN_TILES_V is modified from 24
	u16 voff = x;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff]); voff += VRAM_TILES_H; vram[voff] = pgm_read_byte(&star_map[voff]);
}

void RestoreStarRow(u8 y){
	u16 voff = y*VRAM_TILES_H;
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]);
	vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff++]); vram[voff] = pgm_read_byte(&star_map[voff]);
}



void UpdateMap(){
	if(!(game_state & GS_TITLE_SCREEN)){
		//TorPrint(1,1,PSTR("SCORE["));
		for(u8 i=0;i<=pmax;i++){
			TorPrintInt(8+(i*16),1,pscore[i]);
		}
	}

	AnimateStars();
	if((global_frame&3) == 1)
		AnimateLava();
	u16 voff;

	beam_v_next = 0;//single block every second
	beam_v_wide_next = 23*60;//2 horizontal blocks(starting at 23, then 33,43,53,etc.)
	beam_v_tall_next[0] = 8*60;//2 vertical blocks(these will sometimes sync together..)
	beam_v_tall_next[1] = 18*60;
	beam_v_tall_next[2] = 63*80;//?
	beam_h_next = 30*60;//initial starts at 30, then at 45,60,75,etc.
	island_dissolve_next = 11*60;//dissolves 2 sections of 2 horizontal tiles(on the same island, starts at 11, then 21, 31, etc);

	if(++global_frame > 60000UL)
		global_frame = 0;

	////////////////////////////////////////////////////////////////
	////////Vertical Beam(every 1s), single block drop
	////////////////////////////////////////////////////////////////
	voff = beam_v_x;

	if(beam_v_next == 0){//place block at last position and test hit against player; otherwise setup the new target x/y position
		beam_v_next = 1*60;
		CheckPlayerHitColumn(beam_v_x);
		beam_v_x = 1+(GetPrngNumber(0)%(SCREEN_TILES_H-4));
		for(u8 i=0;i<SCREEN_TILES_V-1;i++){
			if(vram[voff] >= (RAM_TILES_COUNT+FIRST_GROUND_TILE) && vram[voff] <= (RAM_TILES_COUNT+LAST_GROUND_TILE)){
			/////	beam_fast_v_y = i;
				break;
			}
		}
	}else{//update already placed beam
		beam_v_next--;
		u8 beam_v_f = (RAM_TILES_COUNT+FIRST_BEAM_TILE+8)+((global_frame>>1)&7);
		u8 block_pos = 255;
	//	if(global_frame60 >= (60-beam_v_y)){//draw block
	//		block_pos = SCREEN_TILES_V-(60-beam_fast_v_y);
	//	}
	//	for(u8 i=0;i<beam_v_y;i++){
	//		if(i == block_pos)
	//			vram[voff] = RAM_TILES_COUNT+FIRST_GROUND_TILE;//TODO ADD RANDOMNESS..
	//		else
	//			vram[voff] = beam_v_f;
	//		if(++beam_v_f > RAM_TILES_COUNT+LAST_BEAM_TILE)
	//			beam_v_f = RAM_TILES_COUNT+FIRST_BEAM_TILE+8;
	//		voff += VRAM_TILES_H;
	//	}
	}

	////////////////////////////////////////////////////////////////
	////////Island Dissolve(starting at 11, then 21,31,41,etc; 2 x 2wide sections dissolve over 4.5 frames)
	////////////////////////////////////////////////////////////////
	if(island_dissolve_next == 0){//setup x locations, no need to check for hits as player death would be by lava(fall)
		island_dissolve_next = 11*60;
	}else{
		island_dissolve_next--;
		if(island_dissolve_next < ((11-4)*60)-(60/2)){//last dissolve stage, erode 1 tile downwards per frame
			for(u8 i=0;i<2;i++){
				if(island_dissolve_x[i] == 0)//dissolve complete(all tiles gone)
					break;
				voff = 4*VRAM_TILES_H;
				u8 y;
				for(y=4;y<SCREEN_TILES_V;y++){//scan downwards looking for tiles(avoid stars)
					u8 t = vram[voff];
					if(t >= RAM_TILES_COUNT+FIRST_GROUND_TILE && t <= RAM_TILES_COUNT+LAST_GROUND_TILE){
						/*if(t < RAM_TILES_COUNT+FIRST_DISSOLVE_TILE){//must have been dropped after dissolve started...
							u8 t2 = t-(RAM_TILES_COUNT+FIRST_GROUND_TILE);//turn into dissolved 
							t2 *= NUM_DISSOLVE_FRAMES;
							t2 += RAM_TILES_COUNT+FIRST_GROUND_TILE;
							vram[voff] = t2;
						}*/
						u8 t2 = GetPrngNumber(0)%(NUM_STAR_TILES+10);//TODO restore stars correctly from SPI RAM?
						if(t2 >= NUM_STAR_TILES){
							t2 = 0;
						}
						vram[voff] = RAM_TILES_COUNT+FIRST_STAR_TILE+t2;
						break;//only remove 1 tile per frame
					}
					voff += VRAM_TILES_H;
				}
				if(y == SCREEN_TILES_V){//hit bottom with no more ground left?
					island_dissolve_x[0] = 0;//turn off(both)
					break;
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////
	////////Vertical Wide Beam(starting at 23, then 33,43,53; always simultaneous), 2 horizontal blocks
	////////////////////////////////////////////////////////////////
	if(beam_v_wide_next == 0){//place blocks on last tick and test hit against player; otherwise setup the target x/y position
		beam_v_wide_next = 11*60;
	}else{//update already placed beams
		beam_v_wide_next--;
		if(beam_v_wide_x[0]){//not finished?
			u8 f = RAM_TILES_COUNT+FIRST_BEAM_WIDE_TILE+((beam_v_wide_next>>1)&3);
			for(u8 i=0;i<2;i++){
				if(beam_v_wide_next == (11-3)*60){//done, check for player hits
					CheckPlayerHitColumn(beam_v_wide_x[i]+0);
					CheckPlayerHitColumn(beam_v_wide_x[i]+1);
					beam_v_wide_x[i] = 0;//finished
					continue;
				}
				voff = (4*VRAM_TILES_H)+beam_v_wide_x[i];
				for(u8 y=4;y<SCREEN_TILES_H;y++){

				}
				if(beam_v_wide_next < ((11-3)*60)+SCREEN_TILES_V){//draw actual blocks falling
					
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////
	////////Horizontal Beam(starting at 30s then every 15s, lasting 2s), eliminate entire horizontal block row(drawn last)
	////////////////////////////////////////////////////////////////
	if(beam_h_next == 0){//set it up(this is the only beam that overlaps blocks)
		beam_h_next = 15*60;
		beam_h_y = 8+(GetPrngNumber(0)%(SCREEN_TILES_V-9));
	}else if(beam_h_y){//it's active until set to 0
		beam_h_next--;
		if(beam_h_next < 13*60){//finished
			beam_h_y = 0;//stop drawing
			RestoreStarRow(beam_h_y);
		}else{//draw horizontal beam
			u16 voff = beam_h_y*VRAM_TILES_H;
			for(u8 x=0;x<SCREEN_TILES_H;x++){
				u8 t = vram[voff];
				if(t >= RAM_TILES_COUNT+FIRST_GROUND_TILE && t <= RAM_TILES_COUNT+LAST_GROUND_TILE){
					//..
				}else{//TODO keep track of stars?
					//..
				}
				voff++;
			}
		}
	}

	if(++global_frame60 > 59)
		global_frame60 = 0;

return;	
}



void TorPrint(int x,int y,const char *string){
	u8 i=0;
	char c;

	while(1){
		c=pgm_read_byte(&(string[i++]));		
		if(c!=0){
			if(c == ' '){
				x++;
				continue;
			}
			c=((c&127)-28);
			SetFont(x++,y,c);
		}else{
			break;
		}
	}
}



void TorPrintInt(u8 x, u8 y, u16 v){
	if(v > 9999){x++;}
	if(v > 999){x++;}
	if(v > 99){x++;}
	if(v > 9){x++;}

	for(u8 i=0;i<5;i++){
		u8 c = v%10;
		SetTile(x--,y,c+FIRST_NUM_TILE);//65
		v = v/10;
		if(!v)
			break;
	}
}


#define DEV_GENERATE_STAR_MAP 0
#define DEV_GENERATE_GROUND_MAP 0

#if (DEV_GENERATE_STAR_MAP == 1)
void ExportStarMap(){//dump star restore map to EEPROM
	ClearVram();
	GetPrngNumber(0x1ACE);
	DrawMapRLE((SCREEN_TILES_H/2)-2,0,moon_map);

	for(u16 i=0;i<VRAM_SIZE-VRAM_TILES_H;i++){
		if(vram[i] == RAM_TILES_COUNT){
			u8 t = GetPrngNumber(0)%26;
			if(t > (NUM_STAR_TILES/2)-1)
				continue;
			vram[i] = RAM_TILES_COUNT+FIRST_STAR_TILE+t;
		}
	}

	for(u16 i=0;i<VRAM_SIZE-VRAM_TILES_H;i++){//build a raw value table for faster star/moon restores
		WriteEeprom(i, vram[i]);		
	}
	TorPrint(1, 14, PSTR("STAR MAP EXPORTED TO EEPROM"));
	while(1);
}
#endif

#if (DEV_GENERATE_GROUND_MAP == 1)
void ExportGroundMap(){
	ClearVram();
	for(u16 i=0;i<VRAM_SIZE;i++){
		u8 g = GetPrngNumber(0)%NUM_GROUND_TILES;
		g *= (NUM_DISSOLVE_FRAMES+1);
		g += RAM_TILES_COUNT+FIRST_GROUND_TILE;
		vram[i] = g;
		WriteEeprom(i, g);
	}
	TorPrint(1, 14, PSTR("GROUND MAP EXPORTED TO EEPROM"));
	while(1);
}
#endif

int main(){
	GetPrngNumber(GetTrueRandomSeed());
	if(GetPrngNumber(0) == 0)
		GetPrngNumber(0xACE);

	InitMusicPlayer(patches);
	SetMasterVolume(224);
	SetTileTable(tile_data);
	SetSpritesTileBank(0, sprite_data);
	//SetUserPostVsyncCallback(PostVsyncRoutine);
	//SetUserPreVsyncCallback(PreVsyncRoutine);

#if (DEV_GENERATE_STAR_MAP == 1)
	ExportStarMap();
#elif (DEV_GENERATE_GROUND_MAP == 1)
	ExportGroundMap();
#endif
Intro();
MAIN_TOP:
	TitleScreenSetup();
	while(1){
		if(!UpdatePlayers())
			break;
		UpdateMap();
		WaitVsync(1);
	}
	goto MAIN_TOP;
	return 0;
}



void LoadingTransition(){
return;
	FadeOut(1,1);
	SetRenderingParameters(4,4);
	SetRenderingParameters(10,SCREEN_TILES_V*TILE_HEIGHT);
	FadeIn(1,1);
return;
	for(u8 i=0;i<(SCREEN_TILES_V*TILE_HEIGHT)/2;i++){
		SetRenderingParameters(10+i, (SCREEN_TILES_V*TILE_HEIGHT)-(i+10));
		WaitVsync(1);
	}
	for(u8 i=(SCREEN_TILES_V*TILE_HEIGHT)/2;i>=0;i--){
		SetRenderingParameters(10+i, (SCREEN_TILES_V*TILE_HEIGHT)-(i+10));
	}
}



void TitleScreenSetup(){
	game_state |= GS_TITLE_SCREEN;
	ClearVram();
	LoadMap(practice_map);
	DrawMapRLE(0,0,title_map);
	TorPrint((SCREEN_TILES_H/2)-3,17,PSTR("_P GAME"));
	TorPrint((SCREEN_TILES_H/2)-3,18,PSTR("`P COOP"));
	TorPrint((SCREEN_TILES_H/2)-3,19,PSTR("`P VS"));
	if(!(game_state & GS_SONG_LOADED))//if we are just off Intro(), it should be pre-loaded
		LoadSpiRamSong(0);
}



void TitleScreenUpdate(){
	static s8 cursor = 0;
	GetPrngNumber(0);
	last_pad[0] = pad[0];
	pad[0] = ReadJoypad(0);
	if((pad[0] & BTN_SELECT) && !(last_pad[0] & BTN_SELECT)){//move cursor
		//TriggerFx(0,255,1);
		if(++cursor > 2)
			cursor = 0;
		pmax = (cursor>0)?1:0;
		if(cursor < 2)//changed between 1P/2P, reload player(s)
			LoadMap(practice_map);
		if(cursor == 0){
			game_state ^= GS_VERSUS;
		}else if(cursor == 1){
			game_state |= GS_COOP;
		}else if(cursor == 2){//changed to coop
			game_state ^= GS_COOP;
			game_state |= GS_VERSUS;
		}
	}

	for(u8 i=0;i<3;i++)//blank cursor
		SetTile((SCREEN_TILES_H/2)-4,17+i,0);
	SetTile((SCREEN_TILES_H/2)-4,17+cursor,64);

	if((pad[0] & BTN_START) && !(last_pad[0] & BTN_START)){
		game_state ^= GS_TITLE_SCREEN;
		game_state &= ~GS_SONG_LOADED;
		LoadMap(game_map);
		LoadSpiRamSong(1);
		//TriggerFx(2,255,1);
		pstate[0] = STATE_IDLE;
		if(cursor == 1)
			pstate[1] = STATE_IDLE;

		PopulateStars();

		for(u8 i=0;i<=pmax;i++)
			TorPrint(1+(i*16),1,PSTR("SCORE["));
		LoadingTransition();
	}
}