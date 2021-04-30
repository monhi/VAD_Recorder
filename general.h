
#pragma once

#define SUCCESS 0
#define FAILURE -1

typedef				int  int32;
typedef unsigned	int uint32;
typedef			  short  int16;
typedef unsigned  short uint16;
typedef			   char   int8;
typedef unsigned   char  uint8;
typedef float      Float	  ;

#define DEST_FS			8000
#define DEST_BUF_SIZE   DEST_FS/5
#define CHUNK_RATIO  0.20f
#define NN				160

#define		WM_MY_THREAD_MESSAGE	3100
