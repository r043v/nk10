/* PCD8544 aka nokia 3210 & 5110 screen
 * */

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#include <avr/delay.h>

#include <avr/pgmspace.h>

#define nk10_LCD_WIDTH  84
#define nk10_LCD_HEIGHT 48
#define nk10_CHAR_WIDTH 6
#define nk10_CHAR_HEIGHT 8

#define nk10_setOn(p,b) (*p|=(1<<b))
#define nk10_setOff(p,b) (*p&=~(1<<b))

#define nk10_clock_latch {nk10_setOn(m_port_CLK, m_dq_CLK);nk10_setOff(m_port_CLK, m_dq_CLK);}

#define nk10_Port2DDR(p) (p-1)

#define nk10_LCD_CMD 0
#define nk10_LCD_DATA 1

#define nk10_dataMode nk10::enable(nk10_LCD_DATA)
#define nk10_cmdMode nk10::enable(nk10_LCD_CMD)
#define nk10_nbBytes ((nk10_LCD_WIDTH*nk10_LCD_HEIGHT)/8)
#define nk10_topLeft nk10_cmdMode;nk10::write8(0x80);nk10::write8(0x40)
#define nk10_xmoveTo(x) {nk10::write8(0x80|x);}
#define nk10_ymoveTo(y) {nk10::write8(0x40|y);}
#define nk10_move(x,y) {nk10_xmoveTo(x);nk10_ymoveTo(y);}
#define nk10_moveTo(x,y) {nk10_cmdMode;nk10_move(x,y);nk10_dataMode;}
#define nk10_disable nk10_setOn(m_port_SCE,m_dq_SCE)

#define nk10_bit0 0b10000000
#define nk10_bit1 0b01000000
#define nk10_bit2 0b00100000
#define nk10_bit3 0b00010000
#define nk10_bit4 0b00001000
#define nk10_bit5 0b00000100
#define nk10_bit6 0b00000010
#define nk10_bit7 0b00000001

#define nk10_write1(data) {data?nk10_setOn(m_port_DATA,m_dq_DATA):nk10_setOff(m_port_DATA,m_dq_DATA);nk10_clock_latch}
#define nk10_writeb(data,bit) nk10_write1(data&bit)
#define nk10_clear1 nk10_setOff(m_port_DATA,m_dq_DATA);nk10_clock_latch
#define nk10_black1  nk10_setOn(m_port_DATA,m_dq_DATA);nk10_clock_latch

#define nk10_tile4x2(s1,s2) {nk10::write4l(s1);nk10::write4l(s2);\
nk10::write4h(s1++);nk10::write4h(s2++);\
nk10::write4l(s1);nk10::write4l(s2);\
nk10::write4h(s1);nk10::write4h(s2);}

namespace nk10 {


	const u8 bitMask[] = { nk10_bit0, nk10_bit1, nk10_bit2, nk10_bit3, nk10_bit4, nk10_bit5, nk10_bit6, nk10_bit7 };
	
//	u8 x=0,y=0,d=0; // x offset, y line of 8px and y offset in the line
	
	u16 n16; // tmp loop
	u8 n8;
	
		 uint8_t	dq_SCE, dq_RST, dq_DC, dq_DATA, dq_CLK;

		 uint8_t  m_dq_SCE, m_dq_RST, m_dq_DC, m_dq_DATA, m_dq_CLK;
	volatile uint8_t  *m_port_SCE, *m_port_RST, *m_port_DC, *m_port_DATA, *m_port_CLK;
	
	static void enable(u8 isData){
		//nk10_setOn(nk10_Port2DDR(m_port_DATA), m_dq_DATA);
		//nk10_setOn(nk10_Port2DDR(m_port_DC), m_dq_DC);
		
		// Enable display controller (active low)
		nk10_setOff(m_port_SCE, m_dq_SCE);
		isData ? nk10_setOn(m_port_DC,m_dq_DC) : nk10_setOff(m_port_DC,m_dq_DC);
	}
	
	static void commandMode(void){ enable(nk10_LCD_CMD);  }
	static void    dataMode(void){ enable(nk10_LCD_DATA); }
	
/*	void disable(void){
		nk10_setOn(m_port_SCE, m_dq_SCE);
	}*/

	static void clear8(void){
		nk10_clear1;nk10_clear1;nk10_clear1;nk10_clear1;nk10_clear1;nk10_clear1;nk10_clear1;nk10_clear1;
	}
	
	static void clear4(void){
	  nk10_clear1;nk10_clear1;nk10_clear1;nk10_clear1;
	}
	
	static void black8(void){
		nk10_black1;nk10_black1;nk10_black1;nk10_black1;nk10_black1;nk10_black1;nk10_black1;nk10_black1;
	}

	static void write4l(const u8 * data) {
		nk10_writeb(*data,nk10_bit0);
		nk10_writeb(*data,nk10_bit1);
		nk10_writeb(*data,nk10_bit2);
		nk10_writeb(*data,nk10_bit3);
	}

	static void write4h(const u8 * data) {
		nk10_writeb(*data,nk10_bit4);
		nk10_writeb(*data,nk10_bit5);
		nk10_writeb(*data,nk10_bit6);
		nk10_writeb(*data,nk10_bit7);
	}

	static void write4b(const u8 * data, u8 bit, u8 end) {
		while(bit != end) nk10_writeb(*data,bitMask[bit++]);
	}

	static void write8(u8 data){ write4l(&data); write4h(&data); }
	
/*	void tile4x3(u8*s1, u8*s2, u8*s3, u8 dy){
		u8 ddy = 4+dy; n8=2;
		while(1){
			// low bits
			write4b(s1,dy,4); // dy sized, fist colon of first tile
			//clear4();
			write4l(s2); // full sized first colon of tile 2
			write4b(s3,0,dy); // 4-dy sized, fist colon of third tile
			
			// high bits
			write4b(s1,ddy,8); // dy sized, fist colon of first tile
			//clear4();
			write4h(s2); // full sized first colon of tile 2
			write4b(s3,4,ddy); // 4-dy sized, fist colon of third tile
			
			if(!--n8) return;
			s1++; s2++; s3++; // jump byte up
		};
	}
	
	void tile4x2(const u8*s1, const u8*s2){
		// colon1
		write4l(s1);
		write4l(s2);
		// colon2
		write4h(s1++);
		write4h(s2++);
		// colon3
		write4l(s1);
		write4l(s2);
		// colon4
		write4h(s1);
		write4h(s2);
	}
	
	typedef struct map {
		const u8 *map; u8 sx, sy, scx, scy, dx, dy;
		
		//u8 lineStart, lineEnd, colStart, colEnd;
	};*/
	
	const u8 mp[] = { /* [colons][lines] */
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		0,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		1,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		2,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		4,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		5,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		7,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		8,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,

		0,1,2,3,4,5,6,7, 6,5,4,3,2,1,0,2,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,2,3,4,5,6,7, 6,5,4,3,2,1,0,2,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,2,3,4,5,6,7, 6,5,4,3,2,1,0,2,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
		0,1,2,3,4,5,6,7, 6,5,4,3,2,1,0,2,
		0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,

		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6,
		6,5,6,3,6,3,6,3, 6,3,6,3,6,3,6,3,
		3,5,3,6,3,6,3,6, 3,6,3,6,3,6,3,6
	};
	
	const u8 mpsx = 8*6, mpsy = 16;
	
	const u8 * mptr = (const u8 *)mp;
	
/*	map m;
	
	void setMap(map*m, const u8*map, u8 sx, u8 sy, u8 scx, u8 scy){
		m->map = map; m->sx = sx; m->sy = sy; m->scx = scx; m->scy = scy;
	}
	
	void mapblt(map*m){
		
	}*/
	
	const u8 tileset[][2] = {
		{ 0x00, 0x00 }, /* full white */
		{ 0x88, 0x88 }, /* bottom line */
		{ 0x44, 0x44 }, /* third line */
		{ 0x22, 0x22 }, /* second line */
		{ 0x11, 0x11 }, /* top line */
		{ 0x0f, 0x0a },
		{ 0xff, 0xff }, /* full black */
		{ 0x12, 0x48 },
		{ 0x06, 0x60 }
	};
	
//#define s1 colon
//#define s2 (s1+2)
//#define s3 (s1+4)

#define nk10_getTile tileset[*(tile++)]
#define nk10_colJump col+=mpsy;tile=col

static u8 mapByteGet(u8 line, u8 x, u8 y, const u8*m){
	u8 tilex = x >> 2;
	u8 dx = x & 3;
	u8 tiley = y >> 2;
	u8 dy = 4 - (y & 3);
	
	//const u8 *cline = mapptr, *col = cline, *tile = cline;
	
	const u8 *col = m, *tile = m+tilex*mpsy+tiley;
	const u8 *s1, *s2, *s3;
	
	u8 ddy = 4+dy;
	
	s3 = nk10_getTile; s2 = nk10_getTile; s1 = nk10_getTile; nk10_colJump;
	
	// **
	
	// low bits
	write4b(s1,dy,4);
	write4l(s2);
	write4b(s3,0,dy);
	
	// high bits
	write4b(s1,ddy,8);
	write4h(s2);
	write4b(s3,4,ddy);
	
	s1++; s2++; s3++;
	
	// **
	
	// low bits
	write4b(s1,dy,4);
	write4l(s2);
	write4b(s3,0,dy);
	
	// high bits
	write4b(s1,ddy,8);
	write4h(s2);
	write4b(s3,4,ddy);
}

static void ltile4(u8 line, u8 dx, u8 dy, u8 x, u8 sx,const u8 * mapptr){
		nk10_moveTo(x, line >> 4); line &= 15;
		sx -= x+dx;
		const u8 *s1, *s2, *s3;
		const u8 *cline = mapptr, *col = cline, *tile = cline;
		u8 ddx = dx; u8 ddy = 4+dy;
		while(line--){
			if(dx){	// left x clipped colon
				s3 = nk10_getTile; s2 = nk10_getTile; s1 = nk10_getTile; nk10_colJump;
				if(dx==1) {
					write4b(s1,ddy,8); write4h(s2); write4b(s3,4,ddy);
				}
					
				s1++; s2++; s3++;
				
				if(dx<3){
					write4b(s1,dy,4); write4l(s2); write4b(s3,0,dy);
				}
					
				write4b(s1,ddy,8); write4h(s2); write4b(s3,4,ddy);
			}
			
			dx = sx >> 2;
			while(dx--){
				s3 = nk10_getTile; s2 = nk10_getTile; s1 = nk10_getTile; nk10_colJump;
				
				// low bits
				write4b(s1,dy,4);
				write4l(s2);
				write4b(s3,0,dy);
				
				// high bits
				write4b(s1,ddy,8);
				write4h(s2);
				write4b(s3,4,ddy);
				
				s1++; s2++; s3++;
				
				// low bits
				write4b(s1,dy,4);
				write4l(s2);
				write4b(s3,0,dy);
				
				// high bits
				write4b(s1,ddy,8);
				write4h(s2);
				write4b(s3,4,ddy);
			};
			
			dx = sx & 3; // last sprite colon width
			if(dx){	s3 = nk10_getTile; s2 = nk10_getTile; s1 = nk10_getTile; nk10_colJump;
				
				write4b(s1,dy,4); write4l(s2); write4b(s3,0,dy);
				
				if(dx<3){ write4b(s1,ddy,8); write4h(s2); write4b(s3,4,ddy); }
				if(dx==1){ write4b(++s1,dy,4); write4l(++s2); write4b(++s3,0,dy); }
			}
			
			cline += 2; col = cline; tile = cline; dx = ddx;
		};
	}

	static void mapBlt(u8 scx, u8 scy){
		u8 tilex = scx >> 2;
		u8 dx = scx & 3;
		u8 tiley = scy >> 2;
		u8 dy = 4 - (scy & 3);
		ltile4(6,dx,dy,0,84,mptr+tilex*mpsy+tiley);
	}
	
	/*
	#define lpx 84
	#define tpx (lpx*4)
	#define fstart 1
	#define fend 3
	
	/*void sprite4write(u8 x, u8 y, u8 sx, u8 sy, u8*sprite){
		u8 * ptr = (u8*)&sprite;
		if(col & 1) 
		
		col >>= 1; u8 bit = 
		while(size--){
			
		};
	}	

	void sprite4ColWriteb(u8 x, u8 y, u8 sy, u8*sprite){
		u8 bit = x & 1 ? 4 + y : y; if(x > 1) sprite++;
		
		if(bit+sy > 7){
			while(sy--){
				if(bit > 7){ bit -= 8; sprite++; }
				nk10_writeb(*sprite,bit++);
			};
		} else	while(sy--) nk10_writeb(*sprite,bit++);
	}
	
	void spriteLine(u8 start, u8 end, u8 line, u8*sprite, u8 dx, u8 dy){
		nk10_moveTo(start,line);
		u8 sx = end - start;
		u8 firstx = 3 - dx; // first tile width
		u8 fsx = sx - firstx; // total width - first tile width
		u8 nbfullx = fsx >> 2; // full width tiles number
		u8 lastx = fsx&3; // last tile width
		u8 nbfully = dy ? 1 : 2;
		u8 firsty = 3 - dy;
		//u8 lasty = dy;
		
		
		if(dx)		// first tile clipped colon
			for(n8=firstx;n8<4;n8++)
				sprite4ColWriteb(n8,firsty,dy,sprite);
		
		if(lastx)	// last clipped colon
			for(n8=firstx;n8<4;n8++)
				sprite4ColWriteb(n8,firsty,dy,sprite);
	}
	
	void grayscale(void){
		static u16 frame = fstart; nk10_topLeft; nk10_dataMode;
		u16 px8 = frame*lpx;
		n16 = px8; while(n16--) black8();
		n16 = tpx - px8; while(n16--) clear8();
		if(++frame == fend) frame=fstart;
	}*/
	
	void clear(void){
		nk10_topLeft;
		nk10_dataMode;
		n16 = nk10_nbBytes;
		while(n16--) clear8();
	}
	
	void black(void){
		nk10_topLeft;
		nk10_dataMode;
		n16 = nk10_nbBytes;
		while(n16--) black8();
	}
	
	void fill(const u8 *data){
		nk10_topLeft;
		nk10_dataMode;
		n16 = nk10_nbBytes;//( nk10_LCD_WIDTH * nk10_LCD_HEIGHT ) / 8;
		while(n16--) write8(pgm_read_byte(data++));
	}
	
	void log(u8*v,u8 nb){
		//clear();
		nk10_topLeft;
		nk10_dataMode;
		u8 n;
		
		while(nb--){
			n=8; while(n--) if( ((*v) >> n )&1 ) black8(); else clear8(); v++;
		};
		//while(n--) if( ((*v) >> n )&1 ) black8(); else clear8();
	}
	
	/*void tile4(u16 tile1, u16 tile2, u8 px,u8 py){ // 8 lines of 4 pixels
		u8 *t = (u8*)&tile, *end = t+1;
		
		do {
			enable(nk10_LCD_CMD);
			write8(0x80|x);
			write8(0x40|y++);
			
			enable(nk10_LCD_DATA);
			*t & 0b10000000 ? nk10_setOn(m_port_DATA,m_dq_DATA) : nk10_setOff(m_port_DATA,m_dq_DATA);
			nk10_clock_latch
			*t & 0b01000000 ? nk10_setOn(m_port_DATA,m_dq_DATA) : nk10_setOff(m_port_DATA,m_dq_DATA);
			nk10_clock_latch
			*t & 0b00100000 ? nk10_setOn(m_port_DATA,m_dq_DATA) : nk10_setOff(m_port_DATA,m_dq_DATA);
			nk10_clock_latch
			*t & 0b00010000 ? nk10_setOn(m_port_DATA,m_dq_DATA) : nk10_setOff(m_port_DATA,m_dq_DATA);
			nk10_clock_latch
			
			enable(nk10_LCD_CMD);
			write8(0x80|x);
			write8(0x40|y++);
			
			enable(nk10_LCD_DATA);
			*t & 0b00001000 ? nk10_setOn(m_port_DATA,m_dq_DATA) : nk10_setOff(m_port_DATA,m_dq_DATA);
			nk10_clock_latch
			*t & 0b00000100 ? nk10_setOn(m_port_DATA,m_dq_DATA) : nk10_setOff(m_port_DATA,m_dq_DATA);
			nk10_clock_latch
			*t & 0b00000010 ? nk10_setOn(m_port_DATA,m_dq_DATA) : nk10_setOff(m_port_DATA,m_dq_DATA);
			nk10_clock_latch
			*t & 0b00000001 ? nk10_setOn(m_port_DATA,m_dq_DATA) : nk10_setOff(m_port_DATA,m_dq_DATA);
			nk10_clock_latch
		} while(t++ != end);
	}*/
	
/*	void position(u8 px,u8 py){
		x = px; y = py;
		enable(nk10_LCD_CMD);
		write8(0x80|x);
		write8(0x40|y);
		nk10_disable;
	}*/
	
/*	typedef struct sprite {
		u8 x, y, sx, sy, inmap;
	};
	
	typedef struct tile {
		u8 sx, sy, data[2];
	};
	
	typedef struct map {
		u8 *map, x, y, sx, sy, tsx, tsy, scrollx, scrolly, scrolltx, scrollty, scrolld;
		u8 lineStart, lineEnd, colStart, colEnd;
	};
	
	void bpos(u8 px, u8 py){
		x = px; y = py >> 3; d = py % 8; // d = py - y*8;
		
		enable(nk10_LCD_CMD);
		write8(0x80|x);
		write8(0x40|y);
	}
*/	
	/*void blt16(u16 s, u8 px, u8 py){
		bpos(px,py);
		if(d) { enable(nk10_LCD_CMD);
			n8 = d; while(n8--) 
		}
			
			enable(isData);
			write8(data);
			nk10_disable;
		}
	}*/

//const u8 z = 0b10000000;

/*
	u8 ** tiles = [
		[0xf0, 0xcc],
		[0x0d, 0x44]
	];
	
	u8 *mmaparr = [ 0,1,0,1,
			1,1,1,0 ];
	
	map mmap;
	
	void mmapinit(void){
		mmap.map = mmaparr;
		mmap.lineStart = 1;
		mmap.lineEnd = 1;
		mmap.colStart = 12;
		mmap.colEnd = 36;
		mmap.tsx = 4;
		mmap.tsy = 4;
		mmap.sx = 4;
		mmap.sy = 2;
		
		mmap.scrollx = 5;
		mmap.scrolly = 3;
		mmap.scrolltx = 1;
		mmap.scrollty = 0;
		mmap.scrolldy = 3;
		mmap.scrolldx = 1;
	}

	void blitMap(void){//(map*m){
		mmapinit();
		map*m = &mmap;
		
		x = m->colStart; y = m->lineStart; d = 0;
		
		enable(nk10_LCD_CMD); write8(0x80|x); write8(0x40|y);
		//bpos(m->colStart,m->lineStart); if(d) return;
		
		u8 td = m->scrolld; // scroll y % 8
		u8 ty = scrollty; // current y tile
		
		u8 *mpstart = m->map.map, *mp = mpstart + scrollty,
		   *t = tiles[*mp];
		
		u8 lineoffset = 0;
		u8 tileyoffset = m->scrolldy;
		
		u8 *tileydec;
		n8 = 7;
		
		u8 tmask;// = 1 << tileyoffset;
		
		while(n8--){
			tmask = bitMask[tileyoffset++];
			u8 value = *t & tmask;// & tmask; tmask >>= 1;//bitMask[tileyoffset];
			
			if(tileyoffset == m->tsy){ // tile y jump
				tileyoffset = 0;
				mp += m->sx; // jump next line in map
				*t = tiles[*mp]; // get tile
			}
		};
		
		
	}*/
	
	void init(
		volatile uint8_t  *port_SCE, uint8_t  dq_SCE,
		volatile uint8_t  *port_RST, uint8_t  dq_RST,
		volatile uint8_t  *port_DC, uint8_t  dq_DC,
		volatile uint8_t  *port_DATA, uint8_t  dq_DATA,
		volatile uint8_t  *port_CLK, uint8_t  dq_CLK
	)
	{	// save configuration
		m_port_SCE = port_SCE; m_dq_SCE = dq_SCE; 
		m_port_RST = port_RST; m_dq_RST = dq_RST;
		m_port_DC  = port_DC;  m_dq_DC = dq_DC;
		m_port_DATA= port_DATA;m_dq_DATA = dq_DATA;
		m_port_CLK = port_CLK; m_dq_CLK = dq_CLK;
		
		// Pull-up on reset pin
		nk10_setOn(m_port_RST, m_dq_RST);
		
		// Set output bits on lcd port
		nk10_setOn(nk10_Port2DDR(m_port_RST), m_dq_RST);
		nk10_setOn(nk10_Port2DDR(m_port_SCE), m_dq_SCE);
		nk10_setOn(nk10_Port2DDR(m_port_DC), m_dq_DC);
		nk10_setOn(nk10_Port2DDR(m_port_DATA), m_dq_DATA);
		nk10_setOn(nk10_Port2DDR(m_port_CLK), m_dq_CLK);
	    
		// Wait after VCC high for reset (max 30ms)
		_delay_ms(15);
	    
		// Toggle display reset pin
		nk10_setOff(m_port_RST, m_dq_RST);
		_delay_ms(64);
		nk10_setOn(m_port_RST, m_dq_RST);

		nk10_disable;
		
		commandMode();
		
		write8(0x21);  // LCD Extended Commands
		write8(0xC8);  // Set LCD Vop(Contrast)
		write8(0x06);  // Set Temp coefficent
		write8(0x13);  // LCD bias mode 1:48
		write8(0x20);  // Standard Commands, Horizontal addressing
		write8(0x0C);  // LCD in normal mode
		
		clear();
		
		// Set display contrast. Note: No change is visible at ambient temperature 
		int contrast = 0x40;
		write8(0x21); // LCD Extended Commands
		write8(0x80 | contrast); // Set LCD Vop(Contrast)
		write8(0x20); // LCD std cmds, hori addr mode
	}
};
