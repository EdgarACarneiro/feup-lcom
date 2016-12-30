#ifndef __PLANETARY_H
#define __PLANETARY_H

#include "Bitmap.h"
#include "BMPsHolder.h"

#define OK		0

#define GROUND_Y					595

#define NUM_BASES					3
#define CANNON_POS_Y				GROUND_Y - CANNON_SIZE_Y
#define LEFT_CANNON_POS_X			5
#define RIGHT_CANNON_POS_X			795
#define CANNON_PROJECTILE_OFFSET	32

#define HIGHSCORES_TXT				"/home/lcom/svn/lcom1617-t4g01/proj/res/Scores.txt"
#define SCORE_SCORE_X				169
#define SCORE_HOUR_X				297
#define SCORE_MINUTE_X				383
#define SCORE_DAY_X					505
#define SCORE_MONTH_X				595
#define SCORE_YEAR_X				741
#define SCORE_Y						230
#define SCORE_Y_INC					67

typedef enum {
	MENU, GAME_SINGLE, GAME_MULTI, HIGH_SCORES, END_GAME_ANIMATION
} game_state_t;

//TODO: Aqui so leva mesmo esta função ? Rever isto antes da documentação
int timer_handler();

#endif
