#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;

typedef struct tagBITMAPFILEHEADER {

        WORD    bfType;

        DWORD   bfSize;

        WORD    bfReserved1;

        WORD    bfReserved2;

        DWORD   bfOffBits;

} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{

        DWORD      biSize;

        LONG       biWidth;

        LONG       biHeight;

        WORD       biPlanes;

        WORD       biBitCount;

        DWORD      biCompression;

        DWORD      biSizeImage;

        LONG       biXPelsPerMeter;

        LONG       biYPelsPerMeter;

        DWORD      biClrUsed;

        DWORD      biClrImportant;

} BITMAPINFOHEADER;

const int MAX_SIZE = 10000000;
const int COLOR_SIZE = 16;
const int SINGLE_SIZE = 8;

BYTE buffer[MAX_SIZE];
BYTE realBMP[MAX_SIZE];
unsigned char pal[COLOR_SIZE*4];

char reverse(char c)
{
	char out = 0;
	for(int i = 0; i < 8; i++)
		out = (out<<1)+(c&1), c >>= 1;
	return out;
}

void save_pic(char *filename, char *datPath, char *palPath, int width, int height, int row){

	FILE *fd = fopen(datPath,"rb"), *fp = fopen(palPath,"rb");
	fread(buffer,sizeof(BYTE),24,fp); //去掉pal文件头
	unsigned int datSize = fread(buffer,sizeof(BYTE),MAX_SIZE,fd); //读位图数据
	unsigned int palSize = fread(pal,sizeof(BYTE),COLOR_SIZE*4,fp); //读调色板
	fclose(fd),fclose(fp);

	if(height%row) //判断是否能整分
		return;

	int rHeight = row, rWidth = height/row*width;
	rWidth *= SINGLE_SIZE, rHeight *= SINGLE_SIZE;

typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;

        BITMAPFILEHEADER bfh;
        BITMAPINFOHEADER bif;

	//填写BMP文件头
    bfh.bfType=0x4d42;
    bfh.bfSize=14+sizeof(bif)+datSize+palSize; //BMP文件头由于补位所以不能sizeof
    bfh.bfReserved1=0;
    bfh.bfReserved2=0;
    bfh.bfOffBits=14+sizeof(bif)+sizeof(pal);

	//填写位图信息头
    bif.biSize=sizeof(bif);
    bif.biWidth=rWidth;
    bif.biHeight=rHeight;
    bif.biPlanes=1;
    bif.biBitCount=4;
    bif.biCompression=0;
    bif.biSizeImage=datSize;
    bif.biXPelsPerMeter=0;
    bif.biYPelsPerMeter=0;
    bif.biClrUsed=0;
    bif.biClrImportant=0;

    fd=fopen(filename,"wb");

	//写BMP头和位图信息头
	fwrite(&bfh.bfType,sizeof(bfh.bfType),1,fd);
	fwrite(&bfh.bfSize,sizeof(bfh.bfSize),1,fd);
	fwrite(&bfh.bfReserved1,sizeof(bfh.bfReserved1),1,fd);
	fwrite(&bfh.bfReserved2,sizeof(bfh.bfReserved2),1,fd);
	fwrite(&bfh.bfOffBits,sizeof(bfh.bfOffBits),1,fd);
    fwrite(&bif,sizeof(bif),1,fd);

	//写调色板，注意文件中是按B,G,R,0储存
	for(int i = 0; i < COLOR_SIZE; i++)
		putc(pal[(i<<2)+2],fd),putc(pal[(i<<2)+1],fd),putc(pal[(i<<2)+0],fd),putc(pal[(i<<2)+3],fd);
    //fwrite(pal,sizeof(pal),1,fd);

	//按tile和比例要求填写新位图缓冲
	BYTE* ptr = buffer, *now;

	for(int i = 0; i < height; i++)
		for(int j = 0; j < width; j++)
		{
			now = realBMP+(i%row)*(rWidth>>1)*SINGLE_SIZE+((i/row)*(width<<2)+(j*SINGLE_SIZE>>1));
            for(int a = 0; a < SINGLE_SIZE; a++)
				for(int b = 0; b < (SINGLE_SIZE>>1); b++)
					*(now+a*(rWidth>>1)+b) = *(ptr++);
		}

	//将转换后的新位图写入bmp,注意要高低换位
	ptr = realBMP+datSize;

	for(int i=rHeight-1;i>=0;i--)
	{
		now = ptr-(rWidth>>1);
        for(int j=0;j<rWidth;j+=2)
		{
			//char tmp = *now;
			BYTE tmp = (*now>>4)+(((*now)&15)<<4);
			putc(tmp,fd),now++;
		}
		ptr -= rWidth>>1;
     }

	fclose(fd);
}

//将GBA 4bpp的8*8tile位图转换为16色bmp, 参数为 目标文件名 数据文件名 调色板文件名 每行tile数(宽) 每列tile数(高) 输出bmp列tile数(新的高)
int main(int argc, char *argv[])
{
	if(argv[1]==0||argv[2]==0||argv[3]==0||argv[4]==0||argv[5]==0||argv[6]==0)
		return 1;
	save_pic(argv[1],argv[2],argv[3],atoi(argv[4]),atoi(argv[5]),atoi(argv[6]));
	return 0;
}