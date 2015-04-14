#############################################################
# Created:	2013-08-13
# Author:	jianglutian
# Email: 	jianglutian@gmail.com
#
# make		Default, debug mode.
# make clean	Clean is clean, nothing else.
#
#############################################################

# Judged by compiler, you can specify it.
CC 		= gcc

# Object name you want to make, you can change it as you wish.
PROG 	= ../bin/PicTrans/PicTrans.exe

# This is the path for source files, many times "." is enough.
# Though, you can fit with a absolute path.
SRC_DIR	= .

# En, it's the suffix of our source file...
SRC_SFX = .c
OBJ_SFX = .o

# Here, i want to get all source files having the suffix we have specified.
SRC	= $(foreach n, $(wildcard $(addprefix $(SRC_DIR)/*, $(SRC_SFX))), $(n))

# Replace the suffix of SRC.
OBJ 	:= $(SRC:$(SRC_SFX)=$(OBJ_SFX))

# Include files.
INCLUDE = -I../include

# Lib files.
LIB = -L../lib -lDump 

# Debug or Release?
MOD_DEBUG    = -D_DEBUG -g -c -Wall
MOD_RELEASE  = -DNDEBUG -c -Wall -O2

MOD = $(MOD_DEBUG)

$(PROG):$(OBJ)
	$(CC) -o $(PROG) $(OBJ) $(LIB)
$(OBJ):$(SRC)
	$(CC) $(SRC) $(MOD) $(INCLUDE) 


#.PHONY: all install clean distclean core release debug
.PHONY: all clean 
all : $(PROG)

clean :
	rm -f $(PROG) $(OBJ)
	
