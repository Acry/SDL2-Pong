//BEGIN HEAD

//BEGIN DESCRIPTION

/* DESCRIPTION:
 * 
 * This is a simple Pong implementation.
 * 
 * AI, State Integration and timing needs love.
 * But this may be done later with subpixel rendering.
 * 
 * AI-notes:
 * Human rection time (RT) is about 200-300 ms max.
 * Since this AI doesn't interpolate I cheated.
 * RT could be ranged rand, like visibility (range-ratio) could be.
 * Since this is no rocket science it's good for now.
 * 
 * Collision Detection and collision reaction could be upgraded also.
 * 
 * DEFINED PROGRESS GOALS:
 * 
 * FOR THIS VERSION IS SCALING APPLIED
 * Data structures (structs) are declared way more cautious.
 * And since the code is growing (getting more complex), more
 * care was taken style-wise.
 * 
 * Nested Structures are a P.I.T.A TM
 * 
 */

//END   DESCRIPTION

//BEGIN INCLUDES
//system headers
#include <math.h>
//local headers
#include "helper.h"
//END   INCLUDES

//BEGIN CPP DEFINITIONS
#define WHITE 		255,255,255,255
#define BLACK 		0,0,0,255
#define RED   		255,0,0,255
#define BLUE   		0,0,255,255

#define WW 		550
#define WH 		(WW/16)*9

#define RES_W 		1920
#define RES_H 		(RES_W/16)*9

#define END_SCORE 	10
#define PADDLE_SPEED 	1000
#define PADDLE_SIZE	100
#define BALL_SPEED 	800
#define BALL_SIZE	15
#define AFK_TIMER 	8000
#define AI_REACTION 	100
//END   CPP DEFINITIONS

//BEGIN DATASTRUCTURES

struct vec2{
	float 		x;
	float 		y;
};

struct rect{
	struct vec2 	size;
	struct vec2 	pos;
};

struct area_ref{
	struct rect 	frac;
	struct rect 	frac_prev;
	SDL_Rect 	dst;
};

struct entity_static{
	SDL_Texture    *Texture;
	struct rect 	frac;
	SDL_Rect 	dst;
};

struct entity_dyn{
	struct entity_static data;
	struct vec2 	vel;
};

struct ball_{
	struct entity_dyn dyn;
	char dir;
} ball;

struct player{
	struct entity_dyn 	dyn;
	struct entity_static 	ava;
	struct entity_static 	score;
	struct entity_static 	go;

	int 		score_counter;
	bool 		timer;		//state var for afk counter
	Uint32 		afk_time;	//accumulates keypress absence
	bool 		human;		//tells if player is human
};

//END	DATASTRUCTURES

//BEGIN GLOBALS

//BEGIN DISPLAY
int 		ww=WW;
int 		wh=WH;
int 		IARH;  				//Image Aspect Ratio horizontal
int 		IARW;  				//Image Aspect Ratio vertical


struct area_ref	action_area;			//game ganvas
float 		current_scale=1;
//END 	DISPLAY


SDL_Event 	event;
extern 		bool quit;
int 		gameState;
enum 		gameState {IDLE, PLAYING, PAUSED, GAME_OVER};

//BEGIN TIMING
Uint32 		frame_time  = 0;
Uint32 		frame_start = 0;
float 		ms;
//END TIMING

//Oh well...
float 		RACKET_HITBACK_MAXANGLE = 85.0f*M_PI/180.0f;

float 		ball_speed 	= BALL_SPEED;
float 		paddle_speed 	= PADDLE_SPEED;

//BEGIN VISIBLES
SDL_Surface    *temp_surface	= NULL;
TTF_Font       *font 	 	= NULL;

struct entity_static 	cpu_l;
struct entity_static 	cpu_r;

struct entity_static 	logo;

struct entity_static 	shift_l;
struct entity_static 	shift_r;

struct entity_static 	ctrl_l;
struct entity_static 	ctrl_r;

struct entity_static 	lup;
struct entity_static 	ldown;
struct entity_static 	rup;
struct entity_static 	rdown;

struct entity_static 	divider;

struct entity_static 	game_over;
struct entity_static 	paused;

struct player 		p1;
struct player 		p2;
//END 	VISIBLES
Mix_Chunk 	       *hit 	= NULL;
Mix_Chunk 	       *wall 	= NULL;
Mix_Chunk 	       *out 	= NULL;
//END   GLOBALS

//BEGIN FUNCTION PROTOTYPES
//BEGIN SCALING
int  gcd		(int, int);
void scale_ref_area	(void);
void scale_entity_static(struct entity_static *entity);
void scale_all		(void);		//Helper Function to call all Scaling Code
//END SCALING

void score_update	(struct player *p);
void ai1_update		(void);
void ai2_update		(void);
void player_draw_update (void);

void collision		(void);		//also checks score
void ball_bounce	(void);		//flips direction
void ball_reset		(void);
void ball_update	(void);
void bounds_check	(void);

void assets_in		(void);
void assets_out		(void);
void rect_copy		(struct entity_static *s_entity);
void rect_round		(struct entity_static *s_entity);
//BEGIN GAME STATE FUNCTIONS PROTOTYPES
void gameState_check	(void);

/* go drives the high level state called by gameState_check()
 */

//BEGIN IDLE
void IdleGo		(void);
void IdleReady		(void);
void IdleSet		(void);
void IdleUpdate		(void);
void IdleEvents		(void);
void IdleDraw		(void);
void IdleBye		(void);
//END IDLE

//BEGIN PLAYING
void PlayingGo		(void);
void PlayingReady	(void);
void PlayingSet		(void);
void PlayingUpdate	(void);
void PlayingEvents	(void);
void PlayingDraw	(void);
void PlayingBye		(void);
//END 	PLAYING

//BEGIN PAUSED
void PausedGo		(void);
void PausedReady	(void);
void PausedSet		(void);
void PausedUpdate	(void);
void PausedEvents	(void);
void PausedDraw		(void);
void PausedBye		(void);
//END PAUSED

//BEGIN GAME_OVER
void Game_overGo	(void);
void Game_overReady	(void);
void Game_overSet	(void);
void Game_overUpdate	(void);
void Game_overEvents	(void);
void Game_overDraw	(void);
void Game_overBye	(void);
//END GAME_OVER

//END GAME STATE FUNCTIONS PROTOTYPES
//END	FUNCTION PROTOTYPES

//END 	HEAD

//BEGIN MAIN FUNCTION
int main(int argc, char *argv[])
{

(void)argc;
(void)argv;

//BEGIN INIT
if (!openDisplay("PONG", ww, wh))
	return EXIT_FAILURE;

assets_in();
IdleSet();
scale_all();


running=true;
//END   INIT

//BEGIN MAIN LOOP
while(running){
	frame_start=SDL_GetTicks();
	SDL_SetRenderDrawColor(Renderer, WHITE);
	SDL_RenderClear(Renderer);
	gameState_check();
	SDL_RenderPresent(Renderer);
	frame_time = SDL_GetTicks()-frame_start;
	ms=(float)frame_time/1000;
}
//END   MAIN LOOP

assets_out();
closeDisplay();
return EXIT_SUCCESS;

}
//END   MAIN FUNCTION

//BEGIN FUNCTIONS
//BEGIN SCALING
void scale_all(void)
{
	
	scale_ref_area();
	current_scale = action_area.frac.size.x / (float)RES_W;

	scale_entity_static(&logo);
	scale_entity_static(&cpu_l);
	scale_entity_static(&cpu_r);
	scale_entity_static(&shift_l);
	scale_entity_static(&shift_r);
	scale_entity_static(&ctrl_l);
	scale_entity_static(&ctrl_r);
	scale_entity_static(&lup);
	scale_entity_static(&ldown);
	scale_entity_static(&rup);
	scale_entity_static(&rdown);
	scale_entity_static(&divider);
	scale_entity_static(&game_over);
	scale_entity_static(&paused);

	paddle_speed = (float)PADDLE_SPEED * current_scale;

	//BEGIN P1
	scale_entity_static(&p1.dyn.data);
	p1.dyn.vel.x=action_area.frac.size.x * (p1.dyn.vel.x/action_area.frac_prev.size.x);
	p1.dyn.vel.y=action_area.frac.size.y * (p1.dyn.vel.y/action_area.frac_prev.size.y);
	scale_entity_static(&p1.ava);
	scale_entity_static(&p1.score);
	scale_entity_static(&p1.go);
	//END P1
	
	//BEGIN P2
	scale_entity_static(&p2.dyn.data);
	p2.dyn.vel.x=action_area.frac.size.x * (p2.dyn.vel.x/action_area.frac_prev.size.x);
	p2.dyn.vel.y=action_area.frac.size.y * (p2.dyn.vel.y/action_area.frac_prev.size.y);
	scale_entity_static(&p2.ava);
	scale_entity_static(&p2.score);
	scale_entity_static(&p2.go);
	//END 	P2
	
	scale_entity_static(&ball.dyn.data);
	ball_speed   = (float)BALL_SPEED  * current_scale;
	ball.dyn.vel.x=action_area.frac.size.x * (ball.dyn.vel.x/action_area.frac_prev.size.x);
	ball.dyn.vel.y=action_area.frac.size.y * (ball.dyn.vel.y/action_area.frac_prev.size.y);

	



}

void scale_entity_static(struct entity_static *s_entity)
{
	
	//SCALE SIZE
	s_entity->frac.size.y = action_area.frac.size.y * (s_entity->frac.size.y/action_area.frac_prev.size.y);
	s_entity->frac.size.x = action_area.frac.size.x * (s_entity->frac.size.x/action_area.frac_prev.size.x);
	
	s_entity->dst.h=round( s_entity->frac.size.y );
	s_entity->dst.w=round( s_entity->frac.size.x );
	
	//SCALE POSITION
	s_entity->frac.pos.x=( action_area.frac.size.x / ( action_area.frac_prev.size.x / (s_entity->frac.pos.x-action_area.frac_prev.pos.x) ) ) + action_area.frac.pos.x;
	
	s_entity->frac.pos.y=( action_area.frac.size.y / ( action_area.frac_prev.size.y / (s_entity->frac.pos.y-action_area.frac_prev.pos.y) ) ) + action_area.frac.pos.y;
	
	s_entity->dst.x=round( s_entity->frac.pos.x );
	s_entity->dst.y=round( s_entity->frac.pos.y );
}



void scale_ref_area(void)
{
	//SIZE
	action_area.frac_prev.size.x=action_area.frac.size.x;
	action_area.frac_prev.size.y=action_area.frac.size.y;

	if ( (float)wh/(float)IARH  < (float)ww/(float)IARW ){
		action_area.dst.h=wh;
		action_area.frac.size.y=(float)wh;
		action_area.frac.size.x=(action_area.frac.size.y / (float)IARH ) * (float)IARW;
		action_area.dst.w=round( action_area.frac.size.x );
	}else{
		action_area.dst.w=ww;
		action_area.frac.size.x=(float)ww;
		action_area.frac.size.y=(action_area.frac.size.x / (float)IARW ) * (float)IARH;
		action_area.dst.h=round( action_area.frac.size.y );
	}
	
	//pos
	action_area.frac_prev.pos.x=action_area.frac.pos.x;
	action_area.frac_prev.pos.y=action_area.frac.pos.y;
	
	if (action_area.dst.h<=wh){
		action_area.frac.pos.y = (float) wh / 2 - action_area.frac.size.y / 2;
		action_area.dst.y = round(action_area.frac.pos.y);
	}
	
	if (action_area.dst.w<=ww){
		action_area.frac.pos.x = (float) ww / 2 - action_area.frac.size.x / 2;
		action_area.dst.x = round(action_area.frac.pos.x);
	}
	
	SDL_RenderSetClipRect(Renderer, &action_area.dst);
// 	SDL_Log("clip w: %d h: %d x: %d y: %d", action_area.dst.w, action_area.dst.h, action_area.dst.x, action_area.dst.y);
}

int gcd (int a, int b)
{
	
	return (b == 0) ? a : gcd (b, a%b);
	
}
//END SCALING

//BEGIN GAMEPLAY
void collision(void)
{
	if (ball.dyn.data.frac.pos.x<action_area.frac.size.x/4){
		if (SDL_HasIntersection(&ball.dyn.data.dst, &p1.dyn.data.dst)){
			ball.dyn.data.dst.x=p1.dyn.data.dst.x+p1.dyn.data.dst.w+1;
			ball.dyn.data.dst.x=roundf(ball.dyn.data.frac.pos.x);
			ball_bounce();
			Mix_PlayChannel( -1, hit, 0 );
		}
		if (ball.dyn.data.dst.x<=p1.dyn.data.dst.x){
			Mix_PlayChannel( -1, out, 0 );
			p2.score_counter++;
			score_update(&p2);
			ball_reset();
		}
			
	}
	
	if (ball.dyn.data.frac.pos.x>(ww/4)*3){
		if (SDL_HasIntersection(&ball.dyn.data.dst, &p2.dyn.data.dst)){
			ball.dyn.data.dst.x=p2.dyn.data.dst.x-ball.dyn.data.dst.w-1;
			ball.dyn.data.frac.pos.x=ball.dyn.data.dst.x;
			ball_bounce();
			Mix_PlayChannel( -1, hit, 0 );
		}
		if (ball.dyn.data.dst.x>=p2.dyn.data.dst.x+p2.dyn.data.dst.w){
			Mix_PlayChannel( -1, out, 0 );
			p1.score_counter++;
			score_update(&p1);
			ball_reset();
		}
	}
	if ((p1.score_counter == END_SCORE)||(p2.score_counter == END_SCORE))
		Game_overSet();
}

void ai1_update(void)
{
	//player distance
	float dp=p2.dyn.data.frac.pos.x-(p1.dyn.data.frac.pos.x+p1.dyn.data.dst.w);
	//SDL_Log("dp: %f", dp);

	//ball distance
	float db=ball.dyn.data.frac.pos.x-(p1.dyn.data.frac.pos.x+p1.dyn.data.dst.w);
	//SDL_Log("db: %f", db);
	
	//var "visibility"
	float range_ratio = db/dp;
// 	SDL_Log("range_ratio: %f", range_ratio);
	
	float pc = p1.dyn.data.frac.pos.y+(p1.dyn.data.dst.h/2);
	float bc = ball.dyn.data.frac.pos.y+(ball.dyn.data.frac.size.y/2);
	
	float dz=p1.dyn.data.dst.h/3;
	static Uint32 timer = 0;
	timer+=frame_time;
	
	if (timer >= AI_REACTION){
		if (ball.dir==0){
			if (range_ratio < .5){
				if(p1.dyn.data.frac.pos.y+ball.dyn.data.frac.size.y < bc){
					p1.dyn.vel.y =  (paddle_speed*ms);
					timer=0;
					return;
				}
				if(p1.dyn.data.frac.pos.y-ball.dyn.data.frac.size.y > bc){
					p1.dyn.vel.y = -(paddle_speed*ms);
					timer=0;
					return;
				}
				p1.dyn.vel.y=0;
				timer=0;
				return;
			}
		}
		
		//Repositioning
		if (ball.dir==1&&(range_ratio < .5)){
			dz=p1.dyn.data.dst.h/2;
			if (pc < (wh/2)-dz){
				p1.dyn.vel.y =  paddle_speed*ms;
				timer=0;
				return;
			}
			
			if (pc > (wh/2)+dz){
				p1.dyn.vel.y = -(paddle_speed*ms);
				timer=0;
				return;
			
			}
		}
		p1.dyn.vel.y=0;
		timer=0;
	}	

	//Missing: Interpolation of ball position
	//needs other timing and stateintegration

}
void ai2_update(void)
{
	
	//player distance
	float dp=p2.dyn.data.frac.pos.x-(p1.dyn.data.frac.pos.x+p1.dyn.data.dst.w);
// 	SDL_Log("dp: %f", dp);
	
	//ball distance
	float db=p2.dyn.data.frac.pos.x-(ball.dyn.data.frac.pos.x+ball.dyn.data.frac.size.x);

	//var "visibility"
	float range_ratio = db/dp;
// 	SDL_Log("range_ratio: %f", range_ratio);
	
	//Centers
	float pc = p2.dyn.data.frac.pos.y+(p2.dyn.data.dst.h/2);
	float bc = ball.dyn.data.frac.pos.y+(ball.dyn.data.frac.size.y/2);
	
	float dz=p2.dyn.data.dst.h/3;
	static Uint32 timer = 0;
	timer+=frame_time;
	if (timer >= AI_REACTION){
		if (ball.dir==1){
			if (range_ratio < .5){
				if(p2.dyn.data.frac.pos.y+ball.dyn.data.frac.size.y < bc){
					p2.dyn.vel.y =  (paddle_speed*ms);
					timer=0;
					return;
				}
				
				if(p2.dyn.data.frac.pos.y-ball.dyn.data.frac.size.y > bc){
					p2.dyn.vel.y = -(paddle_speed*ms);
					timer=0;
					return;
				}
				
				p2.dyn.vel.y=0;
				timer=0;
				return;
			}
		}
		
		//Repositioning
		if (ball.dir==0&&(range_ratio < .5)){
			//SDL_Log("repos");
			dz=p2.dyn.data.dst.h/2;
			if (pc < (wh/2)-dz){
				p2.dyn.vel.y =  paddle_speed*ms;
				timer=0;
				return;
			}
			if (pc > (wh/2)+dz){
				p2.dyn.vel.y = -(paddle_speed*ms);
				timer=0;
				return;
				
			}
		}
		p2.dyn.vel.y=0;
		timer=0;
	}	
	
	//Missing: Interpolation of ball position
	//needs other timing and stateintegration
	
}

void player_draw_update(void)
{
	p1.dyn.data.frac.pos.y+=p1.dyn.vel.y;
	p1.dyn.data.dst.y=roundf(p1.dyn.data.frac.pos.y);
	
	p2.dyn.data.frac.pos.y+=p2.dyn.vel.y;
	p2.dyn.data.dst.y=roundf(p2.dyn.data.frac.pos.y);

}

void ball_update(void)
{
	//update pos
	ball.dyn.data.frac.pos.x+=ball.dyn.vel.x*ms;
	ball.dyn.data.frac.pos.y+=ball.dyn.vel.y*ms;
	
	
	if (ball.dyn.vel.x>0)
		ball.dyn.data.dst.x=ceilf(ball.dyn.data.frac.pos.x);
	else
		ball.dyn.data.dst.x=floorf(ball.dyn.data.frac.pos.x);
	
	if (ball.dyn.vel.y>0)
		ball.dyn.data.dst.y=ceilf(ball.dyn.data.frac.pos.y);
	else
		ball.dyn.data.dst.y=floorf(ball.dyn.data.frac.pos.y);
	
	if (ball.dyn.vel.x>0)
		ball.dir=1;
	if (ball.dyn.vel.x<0)
		ball.dir=0;

	
}

void bounds_check(void)
{
	if (p1.dyn.data.dst.y<0){
		p1.dyn.data.dst.y=0;
		p1.dyn.data.frac.pos.y=p1.dyn.data.dst.y;
	}
	if(p1.dyn.data.dst.y+p1.dyn.data.dst.h>wh){
		p1.dyn.data.dst.y=wh-p1.dyn.data.dst.h;
		p1.dyn.data.frac.pos.y=p1.dyn.data.dst.y;
	}
	
	if (p2.dyn.data.dst.y<0){
		p2.dyn.data.dst.y=0;
		p2.dyn.data.frac.pos.y=p2.dyn.data.dst.y;
	}
	if(p2.dyn.data.dst.y+p2.dyn.data.dst.h>wh){
		p2.dyn.data.dst.y=wh-p2.dyn.data.dst.h;
		p2.dyn.data.frac.pos.y=p2.dyn.data.dst.y;
	}
	if (ball.dyn.data.dst.y+ball.dyn.data.dst.h>=wh){
		Mix_PlayChannel( -1, wall, 0 );
		ball.dyn.data.dst.y=wh-ball.dyn.data.dst.h-1;
		ball.dyn.data.frac.pos.y=ball.dyn.data.dst.y;
		ball.dyn.vel.y*=-1;
	}
	if (ball.dyn.data.dst.y<=0){
		Mix_PlayChannel( -1, wall, 0 );
		ball.dyn.data.dst.y=1;
		ball.dyn.data.frac.pos.y=ball.dyn.data.dst.y;
		ball.dyn.vel.y*=-1;
	}
}


void ball_bounce(void)
{
	float angle;
	angle = rand()/(float)RAND_MAX * RACKET_HITBACK_MAXANGLE*2.0f -
	RACKET_HITBACK_MAXANGLE;
	ball.dyn.vel.y = sinf(angle)*ball_speed;
	ball.dyn.vel.x *= -1;

}

void ball_reset(void)
{
	float angle;
	
	angle = rand()/(float)RAND_MAX * RACKET_HITBACK_MAXANGLE*2.0f -
	RACKET_HITBACK_MAXANGLE;
	ball.dyn.vel.y = sinf(angle)*ball_speed;
	ball.dyn.vel.x = ball_speed;
	

	ball.dyn.data.frac.pos.x = (action_area.frac.size.x/2)-(ball.dyn.data.frac.size.x/2);
	ball.dyn.data.frac.pos.y = (action_area.frac.size.y/2)-(ball.dyn.data.frac.size.y/2);
	rect_round(&ball.dyn.data);

}
//END 	GAMEPLAY

//BEGIN SCORE
void score_update(struct player *p)
{
	char 			score_string[3];
	SDL_Color 		color={BLUE}; 
	sprintf(score_string, "%d", p->score_counter);
	temp_surface = TTF_RenderText_Solid(font, score_string, color);
	int w,h;
	p->score.Texture  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(p->score.Texture,100);
	SDL_QueryTexture(p->score.Texture, NULL, NULL, &w, &h);
	p->score.frac.size.x=w*current_scale;
	p->score.frac.size.y=h*current_scale;
	p->score.dst.w=roundf(p->score.frac.size.x);
	p->score.dst.h=roundf(p->score.frac.size.y);
}
//END SCORE

//BEGIN GAME STATE FUNCTIONS
void gameState_check(void)
{
	switch(gameState)
	{
		case IDLE:
			IdleGo();
			break;
		case PLAYING:
			PlayingGo();
			break;
		case PAUSED:
			PausedGo();
			break;
		case GAME_OVER:
			Game_overGo();
			break;
		default:
			break;
	}
}
//BEGIN IDLE
void IdleGo(void)
{
	IdleEvents();
	IdleUpdate();
	IdleDraw();
}

void IdleReady(void)
{

	//BEGIN DIVIDER
	divider.frac.size.x = 5;
	divider.frac.size.y = 12;
	divider.frac.pos.x = (action_area.frac.size.x/2)-divider.frac.size.x/2;
	divider.frac.pos.y = divider.frac.size.y;
	//END DIVIDER
	
	p1.dyn.data.frac.size.x = PADDLE_SIZE/10;
	p1.dyn.data.frac.size.y = PADDLE_SIZE;
	p1.dyn.data.frac.pos.x  = 150;
	p1.dyn.data.frac.pos.y  = (action_area.frac.size.y/2)-(p1.dyn.data.frac.size.y/2);
	
	p1.score.frac.pos.x=action_area.frac.size.x/3;
	p1.score.frac.pos.y=20;
	
	p1.timer=false;
	p1.afk_time=0;
		
	p2.dyn.data.frac.size.x = PADDLE_SIZE/10;
	p2.dyn.data.frac.size.y = PADDLE_SIZE;
	p2.dyn.data.frac.pos.x = action_area.frac.size.x-150-p2.dyn.data.frac.size.x;
	p2.dyn.data.frac.pos.y = (action_area.frac.size.y/2)-(p2.dyn.data.frac.size.y/2);
	
	p2.score.frac.pos.x=(action_area.frac.size.x/3)*2;
	p2.score.frac.pos.y=20;
	
	p2.timer=false;
	p2.afk_time=0;

	ball.dyn.data.frac.size.x = BALL_SIZE;
	ball.dyn.data.frac.size.y = BALL_SIZE;
	ball.dyn.data.frac.pos.x = (action_area.frac.size.x/2)-(ball.dyn.data.frac.size.x/2)-ball.dyn.data.frac.size.x/2;
	ball.dyn.data.frac.pos.y = (action_area.frac.size.y/2)-(ball.dyn.data.frac.size.y/2)-ball.dyn.data.frac.size.y/2;

	
}

void IdleSet(void)
{
	p1.dyn.data.frac.pos.y  = (action_area.frac.size.y/2)-(p1.dyn.data.frac.size.y/2);
	rect_round(&p1.dyn.data);
	p1.score_counter=0;
	score_update(&p1);
	
	p2.dyn.data.frac.pos.y = (action_area.frac.size.y/2)-(p2.dyn.data.frac.size.y/2);
	rect_round(&p2.dyn.data);
 	p2.score_counter=0;
 	score_update(&p2);
	
	gameState=IDLE;
}

void IdleUpdate(void)
{
	static Uint32 	timer=0;
	static bool	count=true;
	
	if (count)
		timer+=frame_time;
	
	//After 2 seconds game starts
	if (timer>2000){
		timer=0;
		PlayingSet();
	}
	
}

void IdleEvents(void)
{
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			running = 0;
		}
		if(event.type == SDL_WINDOWEVENT){
			if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
				ww = event.window.data1;
				wh = event.window.data2;
				scale_all();
			}
		}
		if(event.type == SDL_KEYDOWN ){
			switch(event.key.keysym.sym ){
				case SDLK_ESCAPE:
					running = 0;
					break;					
				default:
					break;
			}
		}
	}
}

void IdleDraw(void)
{
	SDL_RenderCopy(Renderer, logo.Texture, NULL, &logo.dst);
	SDL_RenderCopy(Renderer, p2.score.Texture, NULL, &p2.score.dst);
	SDL_RenderCopy(Renderer, p1.score.Texture, NULL, &p1.score.dst);
	
	//BEGIN DEVIDER
	SDL_SetRenderDrawColor(Renderer, BLUE);
	SDL_RenderFillRect(Renderer, &divider.dst);
	float y=divider.frac.pos.y;
	for (int i=0; i<((action_area.frac.size.y/(divider.frac.size.y*2)-1)); i++){
		divider.frac.pos.y+=divider.frac.size.y*2;
		rect_round(&divider);
		SDL_RenderFillRect(Renderer, &divider.dst);
	}
	divider.frac.pos.y=y;
	rect_round(&divider);
	//END 	DEVIDER
	
	SDL_RenderFillRect(Renderer, &p1.dyn.data.dst);
	SDL_RenderFillRect(Renderer, &p2.dyn.data.dst);
	SDL_RenderFillRect(Renderer, &ball.dyn.data.dst);

}

void IdleBye(void)
{
	SDL_DestroyTexture(p1.score.Texture);
	SDL_DestroyTexture(p1.go.Texture);
	SDL_DestroyTexture(p1.ava.Texture);
	SDL_DestroyTexture(p2.score.Texture);
	SDL_DestroyTexture(p2.go.Texture);
	SDL_DestroyTexture(p2.ava.Texture);
}

//END IDLE

//BEGIN PLAYING
void PlayingGo(void)
{
	PlayingEvents();
	PlayingUpdate();
	IdleDraw();
	PlayingDraw();
}

void PlayingReady(void)
{
	int w,h;
	float scale=.75;
	//BEGIN P1 AVA
	temp_surface = IMG_Load("./assets/gfx/p1.png");
	p1.ava.Texture  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(p1.ava.Texture, NULL, NULL, &w, &h);
	p1.ava.frac.size.x=w*scale;
	p1.ava.frac.size.y=h*scale;
	p1.ava.frac.pos.x =2;
	p1.ava.frac.pos.y =(action_area.frac.size.y/2)-(p1.ava.frac.size.y/2);
	//END 	P1 AVA
	
	//BEGIN P2 AVA
	temp_surface = IMG_Load("./assets/gfx/p2.png");
	p2.ava.Texture  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(p2.ava.Texture, NULL, NULL, &w, &h);
	p2.ava.frac.size.x=w*scale;
	p2.ava.frac.size.y=h*scale;
	p2.ava.frac.pos.x =action_area.frac.size.x-2-p2.ava.frac.size.x;
	p2.ava.frac.pos.y =(action_area.frac.size.y/2)-(p2.ava.frac.size.y/2);
	//END 	P2 AVA
	
	//BEGIN CPU L
	temp_surface = IMG_Load("./assets/gfx/cpu.png");
	cpu_l.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(cpu_l.Texture, NULL, NULL, &w, &h);
	
	scale=2;
	cpu_l.frac.size.x=w*scale;
	cpu_l.frac.size.y=h*scale;
	cpu_l.frac.pos.x=2;
	cpu_l.frac.pos.y=(action_area.frac.size.y/2)-(cpu_l.frac.size.y/2);
	//END CPU L
	
	//BEGIN CPU R
	cpu_r.Texture=cpu_l.Texture;
	cpu_r.frac.size.x=w*scale;
	cpu_r.frac.size.y=h*scale;
	cpu_r.frac.pos.x=action_area.frac.size.x-2-cpu_r.frac.size.x;
	cpu_r.frac.pos.y=(action_area.frac.size.y/2)-(cpu_r.frac.size.y/2);
	//END CPU R

	//BEGIN SHIFT L
	temp_surface = IMG_Load("./assets/gfx/shift.png");
	shift_l.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(shift_l.Texture,150);
	SDL_QueryTexture(shift_l.Texture, NULL, NULL, &w, &h);
	scale=1.0;
	shift_l.frac.size.x=w*scale;
	shift_l.frac.size.y=h*scale;
	shift_l.frac.pos.x=2;
	shift_l.frac.pos.y=(action_area.frac.size.y/3)*2;
	//END SHIFT L
	
	//BEGIN SHIFT R
	shift_r.Texture =shift_l.Texture;
	shift_r.frac.size.x=w*scale;
	shift_r.frac.size.y=h*scale;
	shift_r.frac.pos.x=action_area.frac.size.x-2-shift_r.frac.size.x;
	shift_r.frac.pos.y=(action_area.frac.size.y/3)*2;
	//END SHIFT R
	
	//BEGIN CTRL L
	temp_surface = IMG_Load("./assets/gfx/ctrl.png");
	ctrl_l.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(ctrl_l.Texture,150);
	SDL_QueryTexture(ctrl_l.Texture, NULL, NULL, &w, &h);
	ctrl_l.frac.size.x=w*scale;
	ctrl_l.frac.size.y=h*scale;
	ctrl_l.frac.pos.x=2;
	ctrl_l.frac.pos.y=action_area.frac.size.y-2-ctrl_l.frac.size.y;
	//END CTRL L
	
	//BEGIN CTRL R
	ctrl_r.Texture = ctrl_l.Texture;
	ctrl_r.frac.size.x=w*scale;
	ctrl_r.frac.size.y=h*scale;
	ctrl_r.frac.pos.x=action_area.frac.size.x-2-ctrl_r.frac.size.x;
	ctrl_r.frac.pos.y=action_area.frac.size.y-2-ctrl_r.frac.size.y;
	//END CTRL R

	//BEGIN LEFT UP ARROW
	temp_surface = IMG_Load("./assets/gfx/lup.png");
	lup.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(lup.Texture,100);
	SDL_QueryTexture(lup.Texture, NULL, NULL, &w, &h);
	
	lup.frac.size.x=w*scale;
	lup.frac.size.y=h*scale;
	lup.frac.pos.x=4;
	lup.frac.pos.y=shift_l.frac.pos.y+shift_l.frac.size.y+2;
	//END 	LEFT UP ARROW
	
	//BEGIN LEFT DOWN ARROW
	temp_surface = IMG_Load("./assets/gfx/ldown.png");
	ldown.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(ldown.Texture,100);
	SDL_QueryTexture(ldown.Texture, NULL, NULL, &w, &h);
	
	ldown.frac.size.x=w*scale;
	ldown.frac.size.y=h*scale;
	ldown.frac.pos.x=4;
	ldown.frac.pos.y=ctrl_l.frac.pos.y-ldown.frac.size.y;
	//END 	LEFT DOWN ARROW
	
	//BEGIN RIGHT UP ARROW
	temp_surface = IMG_Load("./assets/gfx/rup.png");
	rup.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(rup.Texture,100);
	SDL_QueryTexture(rup.Texture, NULL, NULL, &w, &h);
	
	rup.frac.size.x=w*scale;
	rup.frac.size.y=h*scale;
	rup.frac.pos.x=action_area.frac.size.x-4-rup.frac.size.x;
	rup.frac.pos.y=shift_r.frac.pos.y+shift_r.frac.size.y+2;
	//END 	RIGHT UP ARROW
	
	//BEGIN RIGHT DOWN ARROW
	temp_surface = IMG_Load("./assets/gfx/rdown.png");
	rdown.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(rdown.Texture,100);
	SDL_QueryTexture(rdown.Texture, NULL, NULL, &w, &h);
	
	rdown.frac.size.x=w*scale;
	rdown.frac.size.y=h*scale;
	rdown.frac.pos.x=action_area.frac.size.x-4-rdown.frac.size.x;
	rdown.frac.pos.y=ctrl_r.frac.pos.y-rdown.frac.size.y;
	//END 	RIGHT DOWN ARROW

}

void PlayingSet(void)
{
	ball_reset();
	p1.human=false;
	p2.human=false;
	gameState=PLAYING;
	
}

void PlayingUpdate(void)
{

	if(p1.human==false){
		ai1_update();
	}else if(p1.timer){
		p1.afk_time+=frame_time;
		if (p1.afk_time>=AFK_TIMER)
			p1.human=false;
	}
	if(p2.human==false){
		ai2_update();
	}else if(p2.timer){
		p2.afk_time+=frame_time;
		if (p2.afk_time>=AFK_TIMER)
			p2.human=false;
	}
	
	player_draw_update();
	ball_update();
	bounds_check();
	collision();
}

void PlayingEvents(void)
{
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			running = 0;
		}
		if(event.type == SDL_WINDOWEVENT){
			if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
				ww = event.window.data1;
				wh = event.window.data2;
				scale_all();
			}
		}
		if(event.type == SDL_KEYDOWN ){
			switch(event.key.keysym.sym ){
				case SDLK_ESCAPE:
					running = 0;
					break;	
				case SDLK_p:
					PausedSet();
					break;	
				case SDLK_LSHIFT:
					p1.afk_time=0;
					if (p1.human==false)
						p1.human=true;
					p1.dyn.vel.y = -paddle_speed*ms;
					break;
				case SDLK_LCTRL:
					p1.afk_time=0;
					if (p1.human==false)
						p1.human=true;
					p1.dyn.vel.y = paddle_speed*ms;
					break;
				case SDLK_RSHIFT:
					p2.afk_time=0;
					if (p2.human==false)
						p2.human=true;
					p2.dyn.vel.y = -paddle_speed*ms;
					break;
				case SDLK_RCTRL:
					p2.afk_time=0;
					if (p2.human==false)
						p2.human=true;
					p2.dyn.vel.y = paddle_speed*ms;
					break;
				default:
					break;
			}
		}
		if(event.type == SDL_KEYUP ){
			switch(event.key.keysym.sym ){
				case SDLK_ESCAPE:
					running = 0;
					break;	
					
				case SDLK_LSHIFT:
					p1.timer=true;
					p1.dyn.vel.y = 0;
					break;
				case SDLK_LCTRL:
					p1.timer=true;
					p1.dyn.vel.y = 0;
					break;
				case SDLK_RSHIFT:
					p2.timer=true;
					p2.dyn.vel.y = 0;
					break;
				case SDLK_RCTRL:
					p2.timer=true;
					p2.dyn.vel.y = 0;
					break;
				default:
					break;
			}
		}
		
	}
}

void PlayingDraw(void)
{
	
	if (p1.human==true)
		SDL_RenderCopy(Renderer, p1.ava.Texture, NULL, &p1.ava.dst);
	else
		SDL_RenderCopy(Renderer, cpu_l.Texture, NULL, &cpu_l.dst);
	if (p2.human==true)
		SDL_RenderCopy(Renderer, p2.ava.Texture, NULL, &p2.ava.dst);
	else
		SDL_RenderCopy(Renderer, cpu_r.Texture, NULL, &cpu_r.dst);
	
	SDL_RenderCopy(Renderer, shift_l.Texture, NULL, &shift_l.dst);
	SDL_RenderCopy(Renderer, shift_r.Texture, NULL, &shift_r.dst);
	SDL_RenderCopy(Renderer, ctrl_l.Texture, NULL, &ctrl_l.dst);
	SDL_RenderCopy(Renderer, ctrl_r.Texture, NULL, &ctrl_r.dst);
	
	if (p1.dyn.vel.y<0)
		SDL_RenderCopy(Renderer, lup.Texture, NULL, &lup.dst);
	if (p1.dyn.vel.y>0)	
		SDL_RenderCopy(Renderer, ldown.Texture, NULL, &ldown.dst);
	if (p2.dyn.vel.y<0)
		SDL_RenderCopy(Renderer, rup.Texture, NULL, &rup.dst);
	if (p2.dyn.vel.y>0)	
		SDL_RenderCopy(Renderer, rdown.Texture, NULL, &rdown.dst);
}

void PlayingBye(void)
{

	SDL_DestroyTexture(cpu_l.Texture);
	SDL_DestroyTexture(shift_l.Texture);
	SDL_DestroyTexture(ctrl_l.Texture);
	SDL_DestroyTexture(lup.Texture);
	SDL_DestroyTexture(ldown.Texture);
	SDL_DestroyTexture(rup.Texture);
	SDL_DestroyTexture(rdown.Texture);

}

//END 	PLAYING

//BEGIN PAUSED
void PausedGo(void)
{
	PausedEvents();
	IdleDraw();
	PlayingDraw();
	PausedDraw();
}

void PausedReady(void)
{
	int w,h;
	char 	  string[]={"PAUSED"};
	SDL_Color color={BLUE}; 
	temp_surface = TTF_RenderText_Solid(font, string, color);
	paused.Texture  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(paused.Texture, NULL, NULL, &w, &h);
	paused.frac.size.x=w;
	paused.frac.size.y=h;
	paused.frac.pos.x=(action_area.frac.size.x/2)-(paused.frac.size.x/2);
	paused.frac.pos.y=(action_area.frac.size.y/2)-(paused.frac.size.y/2);
}

void PausedSet(void)
{
	gameState=PAUSED;
}

void PausedEvents(void)
{
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			running = 0;
		}
		if(event.type == SDL_WINDOWEVENT){
			if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
				ww = event.window.data1;
				wh = event.window.data2;
				scale_all();
			}
		}
		if(event.type == SDL_KEYDOWN ){
			switch(event.key.keysym.sym ){
				case SDLK_ESCAPE:
					running = 0;
					break;	
				default:
				gameState=PLAYING;
			}
		}
	}
}

void PausedDraw(void)
{
	
SDL_RenderCopy(Renderer, paused.Texture, NULL, &paused.dst);
	
}

void PausedBye(void)
{
	SDL_DestroyTexture(paused.Texture);
}

//END PAUSED

//BEGIN GAME_OVER
void Game_overGo(void)
{
	Game_overEvents();
	Game_overUpdate();
	Game_overDraw();
}

void Game_overReady(void)
{
	int w,h;
	SDL_Color 	color={BLUE}; 
	char 		string[]={"GAME OVER"};
	
	temp_surface = TTF_RenderText_Solid(font, string, color);
	game_over.Texture  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(game_over.Texture, NULL, NULL, &w, &h);
	game_over.frac.size.x=w;
	game_over.frac.size.y=h;
	game_over.frac.pos.x=action_area.frac.size.x/2-game_over.frac.size.x/2;
	game_over.frac.pos.y=game_over.frac.size.y*2;
	
	char 		string1[]={"Player 1 won"};
	temp_surface = TTF_RenderText_Solid(font, string1, color);
	p1.go.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(p1.go.Texture, NULL, NULL, &w, &h);
	p1.go.frac.size.x=w;
	p1.go.frac.size.y=h;
	p1.go.frac.pos.x=action_area.frac.size.x/2-p1.go.frac.size.x/2-14;
	p1.go.frac.pos.y=action_area.frac.size.y/2+p1.go.frac.size.y/2;
	
	char 		string2[]={"Player 2 won"};
	
	temp_surface = TTF_RenderText_Solid(font, string2, color);
	p2.go.Texture = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(p2.go.Texture, NULL, NULL, &w, &h);
	p2.go.frac.size.x=w;
	p2.go.frac.size.y=h;
	p2.go.frac.pos.x=action_area.frac.size.x/2-p2.go.frac.size.x/2-14;
	p2.go.frac.pos.y=action_area.frac.size.y/2+p2.go.frac.size.y/2;
	
}

void Game_overSet(void)
{
	gameState=GAME_OVER;
}

void Game_overUpdate(void)
{
	static Uint32 	timer=0;
	static bool	count=true;
	
	if (count)
		timer+=frame_time;
	
	//After 5 seconds go back to Idle
	if (timer>5000){
		timer=0;
		IdleSet();
	}
}

void Game_overEvents(void)
{
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			running = 0;
		}
		if(event.type == SDL_WINDOWEVENT){
			if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
				ww = event.window.data1;
				wh = event.window.data2;
				scale_all();
			}
		}
		if(event.type == SDL_KEYDOWN ){
			switch(event.key.keysym.sym ){
				case SDLK_ESCAPE:
					running = 0;
					break;					
				default:
					break;
			}
		}
	}
}

void Game_overDraw(void)
{
	IdleDraw();
	SDL_RenderCopy(Renderer, game_over.Texture, NULL, &game_over.dst);
	if(p1.score_counter==END_SCORE)
		SDL_RenderCopy(Renderer, p1.go.Texture, NULL, &p1.go.dst);
	else
		SDL_RenderCopy(Renderer, p2.go.Texture, NULL, &p2.go.dst);
}

void Game_overBye(void)
{
	SDL_DestroyTexture(game_over.Texture);
}

//END 	GAME_OVER

//END 	GAME STATE FUNCTIONS

//BEGIN HELPER
void assets_in(void)
{

	hit  = Mix_LoadWAV("./assets/phit.oga");
	Mix_VolumeChunk(hit, 35);
	wall = Mix_LoadWAV("./assets/wall.oga");
	Mix_VolumeChunk(wall, 35);
	out = Mix_LoadWAV("./assets/score.oga");
	Mix_VolumeChunk(out, 35);
	font = TTF_OpenFont("./assets/8-bit.ttf", 72);
	
	//BEGIN ACTION AREA
	action_area.frac.size.x=RES_W;
	action_area.frac.size.y=RES_H;
	action_area.frac.pos.x=0;
	action_area.frac.pos.y=0;
	//END ACTION AREA

	//BEGIN SCALING
	int gcd_;
	gcd_= gcd(action_area.frac.size.x,action_area.frac.size.y);
	IARH=action_area.frac.size.y/gcd_;
	IARW=action_area.frac.size.x/gcd_;
	SDL_Log("Aspect Ratio: %d:%d",IARW,IARH);
	//END SCALING
	
	//BEGIN LOGO
	temp_surface = IMG_Load("./assets/gfx/logo_big.png");
	logo.Texture = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	int w,h;
	SDL_SetTextureAlphaMod(logo.Texture,100);
	SDL_QueryTexture(logo.Texture, NULL, NULL, &w, &h);
	logo.frac.size.x=w;
	logo.frac.size.y=h;
	logo.frac.pos.x=action_area.frac.size.x/2-logo.frac.size.x/2;
	logo.frac.pos.y=action_area.frac.size.y/2-logo.frac.size.y/2;

	//END 	LOGO
	
	IdleReady();
	PlayingReady();
	PausedReady();
	Game_overReady();
}

void assets_out(void)
{
	SDL_FreeSurface(temp_surface);
	SDL_DestroyTexture(logo.Texture);
	Mix_FreeChunk(hit);
	Mix_FreeChunk(wall);
	Mix_FreeChunk(out);
	TTF_CloseFont(font);
	IdleBye();
	PlayingBye();
	PausedBye();
	Game_overBye();
}

void rect_copy(struct entity_static *s_entity)
{
	s_entity->frac.pos.x=s_entity->dst.x;
	s_entity->frac.pos.y=s_entity->dst.y;
	s_entity->frac.size.x=s_entity->dst.w;
	s_entity->frac.size.y=s_entity->dst.h;
}

void rect_round(struct entity_static *s_entity)
{
	s_entity->dst.x=roundf(s_entity->frac.pos.x);
	s_entity->dst.y=roundf(s_entity->frac.pos.y);
	s_entity->dst.w=roundf(s_entity->frac.size.x)    ;
	s_entity->dst.h=roundf(s_entity->frac.size.y)    ;
}
//END 	HELPER
//END   FUNCTIONS
