//BEGIN HEAD

//BEGIN DESCRIPTION

/* DESCRIPTION:
 * 
 * This is a simple pong implementation.
 * AI, State Integration and timing needs love.
 * But this may be done later with subpixel rendering.
 * 
 * AI-notes:
 * Human rection time (RT) is about 200-300 ms max.
 * Since this AI doesn't interpolate I cheated.
 * RT could be ranged rand, like visibility could.
 * But since this is no rocket science it's good for now.
 * 
 * DEFINED PROGRESS GOALS:
 * 
 * NEXT VERSION: SCALING APPLIED
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
#define WH 		(WW/16)*12

#define END_SCORE 	10
#define PADDLE_SPEED 	800
#define BALL_SPEED 	300
#define AFK_TIMER 	4500
#define AI_REACTION 	150
//END   CPP DEFINITIONS

//BEGIN DATASTRUCTURES
struct State
{
	float x;
	float y;
	float vx;
	float vy;
};

struct PongBall {
	struct State cur;
	SDL_Rect dst;
	char dir;
}Ball;

struct player{
	struct State 	cur;		//pos
	SDL_Rect 	dst;
	SDL_Texture    *ava;		//human player pic
	SDL_Rect 	ava_dst;
	SDL_Texture    *tex;		//score count tex
	SDL_Rect 	score_dst;
	SDL_Texture    *go;		//player won tex
	SDL_Rect 	go_dst;
	int 		score;
	bool 		timer;		//state var for afk counter
	Uint32 		afk_time;	//accumulates keypress absence
	bool 		human;		//tells if player is human
};
//END	DATASTRUCTURES

//BEGIN GLOBALS
int 	ww=WW;
int 	wh=WH;

SDL_Event event;
extern 	bool quit;
int 	gameState;
enum 	gameState {IDLE, PLAYING, PAUSED, GAME_OVER};

//BEGIN TIMING
Uint32 frame_time  = 0;
Uint32 frame_start = 0;
float ms;
//END TIMING
//Oh well, this is premature...but "good enough" atm
float 		RACKET_HITBACK_MAXANGLE = 85.0f*M_PI/180.0f;

float 		ball_speed 	= BALL_SPEED;
float 		paddle_speed 	= PADDLE_SPEED;

//BEGIN VISIBLES
SDL_Surface    *temp_surface	= NULL;
TTF_Font       *font 	 	= NULL;

SDL_Texture    *cpu		= NULL;
SDL_Rect 	cpu_dst;
SDL_Rect 	cpu_dst2;

SDL_Texture    *logo		= NULL;
SDL_Rect 	logo_dst;

SDL_Texture    *shift		= NULL;
SDL_Rect 	shift_dst;
SDL_Rect 	shift_dst2;

SDL_Texture    *ctrl		= NULL;
SDL_Rect 	ctrl_dst;
SDL_Rect 	ctrl_dst2;

SDL_Texture    *ldown		= NULL;
SDL_Rect 	ldown_dst;
SDL_Texture    *lup		= NULL;
SDL_Rect 	lup_dst;
SDL_Texture    *rdown		= NULL;
SDL_Rect 	rdown_dst;
SDL_Texture    *rup		= NULL;
SDL_Rect 	rup_dst;

SDL_Rect 	divider;
SDL_Texture    *game_over	= NULL;
SDL_Rect 	game_over_dst;
SDL_Texture    *paused		= NULL;
SDL_Rect 	paused_dst;

struct player 	p1;
struct player 	p2;
//END 	VISIBLES
Mix_Chunk 	*hit 		= NULL;
Mix_Chunk 	*wall 		= NULL;
//END   GLOBALS

//BEGIN FUNCTION PROTOTYPES
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
//BEGIN GAME STATE FUNCTIONS PROTOTYPES
void gameState_check	(void);

/* go drives the high level state called by gameState_check()
 */

//BEGIN IDLE
void IdleGo(void);
void IdleReady(void);
void IdleSet(void);
void IdleUpdate(void);
void IdleEvents(void);
void IdleDraw(void);
void IdleBye(void);
//END IDLE

//BEGIN PLAYING
void PlayingGo(void);
void PlayingReady(void);
void PlayingSet(void);
void PlayingUpdate(void);
void PlayingEvents(void);
void PlayingDraw(void);
void PlayingBye(void);
//END 	PLAYING

//BEGIN PAUSED
void PausedGo(void);
void PausedReady(void);
void PausedSet(void);
void PausedUpdate(void);
void PausedEvents(void);
void PausedDraw(void);
void PausedBye(void);
//END PAUSED

//BEGIN GAME_OVER
void Game_overGo(void);
void Game_overReady(void);
void Game_overSet(void);
void Game_overUpdate(void);
void Game_overEvents(void);
void Game_overDraw(void);
void Game_overBye(void);
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

//BEGIN BALL
void collision(void)
{
	if (Ball.cur.x<ww/4){
		if (SDL_HasIntersection(&Ball.dst, &p1.dst)){
			Ball.dst.x=p1.dst.x+p1.dst.w+1;
			Ball.cur.x=Ball.dst.x;
			ball_bounce();
			Mix_PlayChannel( -1, hit, 0 );
		}
		if (Ball.dst.x<=p1.dst.x){
			p2.score++;
			score_update(&p2);
			ball_reset();
		}
			
	}
	
	if (Ball.cur.x>(ww/4)*3){
		if (SDL_HasIntersection(&Ball.dst, &p2.dst)){
			Ball.dst.x=p2.dst.x-Ball.dst.w-1;
			Ball.cur.x=Ball.dst.x;
			ball_bounce();
			Mix_PlayChannel( -1, hit, 0 );
		}
		if (Ball.dst.x>=p2.dst.x+p2.dst.w){
			p1.score++;
			score_update(&p1);
			ball_reset();
		}
	}
	if ((p1.score == END_SCORE)||(p2.score == END_SCORE))
		Game_overSet();
}

void ai1_update(void)
{
	//player distance
	float dp=p2.cur.x-(p1.cur.x+p1.dst.w);
	//SDL_Log("dp: %f", dp);

	//ball distance
	float db=Ball.cur.x-(p1.cur.x+p1.dst.w);
	//SDL_Log("db: %f", db);
	
	//var "visibility"
	float range_ratio = db/dp;
	
	float pc = p1.cur.y+(p1.dst.h/2);
	float bc = Ball.cur.y+(Ball.dst.h/2);
	
	float dz=p1.dst.h/3;
	static Uint32 timer = 0;
	timer+=frame_time;
	
	if (timer >= AI_REACTION){
// 		SDL_Log("range: %f", range);
		if (Ball.dir==0){
			if (range_ratio < .5){
				if(p1.cur.y+Ball.dst.h < bc){
					p1.cur.vy =  (paddle_speed*ms);
					timer=0;
					return;
				}
				if(p1.cur.y-Ball.dst.h > bc){
					p1.cur.vy = -(paddle_speed*ms);
					timer=0;
					return;
				}

				p1.cur.vy=0;
				timer=0;
				return;
			}
		}
		
		//Repositioning
		if (Ball.dir==1&&(range_ratio < .5)){
			dz=p1.dst.h/2;
			if (pc < (wh/2)-dz){
				p1.cur.vy =  paddle_speed*ms;
				timer=0;
				return;
			}
			if (pc > (wh/2)+dz){
				p1.cur.vy = -(paddle_speed*ms);
				timer=0;
				return;
			
			}
		}
		p1.cur.vy=0;
		timer=0;
	}	

	//Missing: Interpolation of ball position
	//needs other timing and stateintegration

}
void ai2_update(void)
{
	//player distance
	float dp=p2.cur.x-(p1.cur.x+p1.dst.w);
	//SDL_Log("dp: %f", dp);
	
	//ball distance
	float db=p2.cur.x-(Ball.cur.x+Ball.dst.w);
	//SDL_Log("db: %f", db);
	
	//var "visibility"
	float range_ratio = db/dp;
	
	float pc = p2.cur.y+(p2.dst.h/2);
	float bc = Ball.cur.y+(Ball.dst.h/2);
	
	float dz=p2.dst.h/3;
	static Uint32 timer = 0;
	timer+=frame_time;
	
	if (timer >= AI_REACTION){
// 		SDL_Log("range: %f", range);
		if (Ball.dir==1){
			if (range_ratio < .5){
				//SDL_Log("vis");
				
				if(p2.cur.y+Ball.dst.h < bc){
					p2.cur.vy =  (paddle_speed*ms);
					timer=0;
					return;
				}
				
				if(p2.cur.y-Ball.dst.h > bc){
					p2.cur.vy = -(paddle_speed*ms);
					timer=0;
					return;
				}
				
				p2.cur.vy=0;
				timer=0;
				return;
			}
		}
		
		//Repositioning
		if (Ball.dir==0&&(range_ratio < .5)){
			//SDL_Log("repos");
			dz=p2.dst.h/2;
			if (pc < (wh/2)-dz){
				p2.cur.vy =  paddle_speed*ms;
				timer=0;
				return;
			}
			
			if (pc > (wh/2)+dz){
				p2.cur.vy = -(paddle_speed*ms);
				timer=0;
				return;
				
			}
		}
		p2.cur.vy=0;
		timer=0;
	}	
	
	//Missing: Interpolation of ball position
	//needs other timing and stateintegration
	
}

//NOT USED ATM
void ai3_update(void)
{

	if(p2.dst.y + 0.5 * p2.dst.h <= Ball.cur.y + 0.5 * Ball.dst.h)
		p2.cur.vy = paddle_speed*ms;
	else{
		p2.cur.vy = -paddle_speed*ms;
	}

}

void player_draw_update(void)
{
	p1.cur.y+=p1.cur.vy;
	p1.dst.y=roundf(p1.cur.y);
	
	p2.cur.y+=p2.cur.vy;
	p2.dst.y=roundf(p2.cur.y);

}

void ball_update(void)
{
	//update pos
	Ball.cur.x+=Ball.cur.vx*ms;
	Ball.cur.y+=Ball.cur.vy*ms;
	
	
	if (Ball.cur.vx>0)
		Ball.dst.x=ceilf(Ball.cur.x);
	else
		Ball.dst.x=floorf(Ball.cur.x);
	
	if (Ball.cur.vy>0)
		Ball.dst.y=ceilf(Ball.cur.y);
	else
		Ball.dst.y=floorf(Ball.cur.y);
	
	if (Ball.cur.vx>0)
		Ball.dir=1;
	if (Ball.cur.vx<0)
		Ball.dir=0;

	
}

void bounds_check(void)
{
	if (p1.dst.y<0){
		p1.dst.y=0;
		p1.cur.y=p1.dst.y;
	}
	if(p1.dst.y+p1.dst.h>wh){
		p1.dst.y=wh-p1.dst.h;
		p1.cur.y=p1.dst.y;
	}
	
	if (p2.dst.y<0){
		p2.dst.y=0;
		p2.cur.y=p2.dst.y;
	}
	if(p2.dst.y+p2.dst.h>wh){
		p2.dst.y=wh-p2.dst.h;
		p2.cur.y=p2.dst.y;
	}
	if (Ball.dst.y+Ball.dst.h>=wh){
		Mix_PlayChannel( -1, wall, 0 );
		Ball.dst.y=wh-Ball.dst.h-1;
		Ball.cur.y=Ball.dst.y;
		Ball.cur.vy*=-1;
	}
	if (Ball.dst.y<=0){
		Mix_PlayChannel( -1, wall, 0 );
		Ball.dst.y=1;
		Ball.cur.y=Ball.dst.y;
		Ball.cur.vy*=-1;
	}
}


void ball_bounce(void)
{
	float angle;
	angle = rand()/(float)RAND_MAX * RACKET_HITBACK_MAXANGLE*2.0f -
	RACKET_HITBACK_MAXANGLE;
	Ball.cur.vy = sinf(angle)*ball_speed;
	Ball.cur.vx *= -1;

}

void ball_reset(void)
{
	float angle;
	
	angle = rand()/(float)RAND_MAX * RACKET_HITBACK_MAXANGLE*2.0f -
	RACKET_HITBACK_MAXANGLE;
	Ball.cur.vy = sinf(angle)*ball_speed;
	Ball.cur.vx = ball_speed;
	
	Ball.dst.x = ww/2 - Ball.dst.w/2.0f;
	Ball.dst.y = wh/2 - Ball.dst.h/2.0f;
	Ball.cur.x = Ball.dst.x;
	Ball.cur.y = Ball.dst.y;

}
//END 	BALL

//BEGIN SCORE
void score_update(struct player *p)
{
	char 			score_string[3];
	SDL_Color 		color={BLUE}; 
	sprintf(score_string, "%d", p->score);
	temp_surface = TTF_RenderText_Solid(font, score_string, color);
	p->tex  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(p->tex,100);
	SDL_QueryTexture(p->tex, NULL, NULL, &p->score_dst.w, &p->score_dst.h);

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
	divider.w = 2;
	divider.h = 8;
	divider.x = (ww/2)-1;
	divider.y = 4;
	
	p1.dst.w = 5;
	p1.dst.h = 50;
	p1.dst.x = 50;
	p1.cur.x = p1.dst.x;
	p1.dst.y = (wh/2)-(p1.dst.h/2);
	p1.cur.y = p1.dst.y;
	
	p1.score_dst.x=(ww/2)-50;
	p1.score_dst.y=20;
	p1.timer=false;
	p1.afk_time=0;
		
	p2.dst.w = 5;
	p2.dst.h = 50;
	p2.dst.x = ww-50-p2.dst.w;
	p2.cur.x = p2.dst.x;
	p2.dst.y = (wh/2)-(p2.dst.h/2);
	
	p2.score_dst.x=(ww/2)+20;
	p2.score_dst.y=20;
	p2.timer=false;
	p2.afk_time=0;

	Ball.dst.w = 8;
	Ball.dst.h = 8;
	Ball.dst.x = (ww/2)-(Ball.dst.w/2);
	Ball.dst.y = (wh/2)-(Ball.dst.h/2);
	Ball.cur.y=Ball.dst.y;
	Ball.cur.x=Ball.dst.x;
	
}

void IdleSet(void)
{
	p1.dst.y = (wh/2)-(p1.dst.h/2);
	p1.cur.y = p1.dst.y;
	p2.dst.y = (wh/2)-(p2.dst.h/2);
	p2.cur.y = p2.dst.y;
	
	p1.score=0;
 	p2.score=0;
	score_update(&p1);
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
	SDL_RenderCopy(Renderer, logo, NULL, &logo_dst);
	SDL_RenderCopy(Renderer, p2.tex, NULL, &p2.score_dst);
	SDL_RenderCopy(Renderer, p1.tex, NULL, &p1.score_dst);
	
	//BEGIN DEVIDER
	SDL_SetRenderDrawColor(Renderer, BLUE);
	SDL_RenderFillRect(Renderer, &divider);
	for (int i=0; i<((wh/(divider.h*2)-1)); i++){
		divider.y+=divider.h*2;
		SDL_RenderFillRect(Renderer, &divider);
	}
	divider.y=4;
	//END 	DEVIDER
	
	SDL_RenderFillRect(Renderer, &p1.dst);
	SDL_RenderFillRect(Renderer, &p2.dst);
	SDL_RenderFillRect(Renderer, &Ball.dst);

}

void IdleBye(void)
{
	SDL_DestroyTexture(p1.tex);
	SDL_DestroyTexture(p1.go);
	SDL_DestroyTexture(p1.ava);
	SDL_DestroyTexture(p2.tex);
	SDL_DestroyTexture(p2.go);
	SDL_DestroyTexture(p2.ava);
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
	temp_surface = IMG_Load("./assets/gfx/p1.png");
	p1.ava  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(p1.ava, NULL, NULL, &w, &h);
	p1.ava_dst.w=w*.25;
	p1.ava_dst.h=h*.25;
	p1.ava_dst.x=2;
	p1.ava_dst.y=(wh/2)-(p1.ava_dst.h/2);
	
	temp_surface = IMG_Load("./assets/gfx/p2.png");
	p2.ava  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(p2.ava, NULL, NULL, &w, &h);
	p2.ava_dst.w=w*.25;
	p2.ava_dst.h=h*.25;
	p2.ava_dst.x=ww-2-p2.ava_dst.w;
	p2.ava_dst.y=(wh/2)-(p2.ava_dst.h/2);
	
	temp_surface = IMG_Load("./assets/gfx/cpu.png");
	cpu = SDL_CreateTextureFromSurface(Renderer,temp_surface );

	SDL_QueryTexture(cpu, NULL, NULL, &w, &h);
	float scale=0.5;
	cpu_dst.w=w*scale;
	cpu_dst.h=h*scale;
	cpu_dst.x=2;
	cpu_dst.y=(wh/2)-(cpu_dst.h/2);
	
	cpu_dst2.w=w*scale;
	cpu_dst2.h=h*scale;
	cpu_dst2.x=ww-2-cpu_dst.w;
	cpu_dst2.y=(wh/2)-(cpu_dst.h/2);
	
	temp_surface = IMG_Load("./assets/gfx/shift.png");
	shift = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(shift,150);
	SDL_QueryTexture(shift, NULL, NULL, &w, &h);
	scale=0.4;
	shift_dst.w=w*scale;
	shift_dst.h=h*scale;
	shift_dst.x=2;
	shift_dst.y=(wh/3)*2;
	
	shift_dst2.w=w*scale;
	shift_dst2.h=h*scale;
	shift_dst2.x=ww-2-shift_dst.w;
	shift_dst2.y=(wh/3)*2;
	
	temp_surface = IMG_Load("./assets/gfx/ctrl.png");
	ctrl = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(ctrl,150);
	SDL_QueryTexture(ctrl, NULL, NULL, &w, &h);
	scale=0.4;
	ctrl_dst.w=w*scale;
	ctrl_dst.h=h*scale;
	ctrl_dst.x=2;
	ctrl_dst.y=wh-2-ctrl_dst.h;
	
	ctrl_dst2.w=w*scale;
	ctrl_dst2.h=h*scale;
	ctrl_dst2.x=ww-2-ctrl_dst.w;
	ctrl_dst2.y=wh-2-ctrl_dst.h;
	
	temp_surface = IMG_Load("./assets/gfx/rup.png");
	rup = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(rup,100);
	SDL_QueryTexture(rup, NULL, NULL, &w, &h);
	scale=0.3;
	rup_dst.w=w*scale;
	rup_dst.h=h*scale;
	rup_dst.x=ww-4-rup_dst.w;
	rup_dst.y=shift_dst2.y+shift_dst2.h+2;
	
	temp_surface = IMG_Load("./assets/gfx/rdown.png");
	rdown = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(rdown,100);
	SDL_QueryTexture(rdown, NULL, NULL, &w, &h);

	rdown_dst.w=w*scale;
	rdown_dst.h=h*scale;
	rdown_dst.x=ww-4-rdown_dst.w;
	rdown_dst.y=ctrl_dst.y-rdown_dst.h;
	
	temp_surface = IMG_Load("./assets/gfx/lup.png");
	lup = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(lup,100);
	SDL_QueryTexture(lup, NULL, NULL, &w, &h);

	lup_dst.w=w*scale;
	lup_dst.h=h*scale;
	lup_dst.x=4;
	lup_dst.y=shift_dst.y+shift_dst.h+2;
	
	temp_surface = IMG_Load("./assets/gfx/ldown.png");
	ldown = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_SetTextureAlphaMod(ldown,100);
	SDL_QueryTexture(ldown, NULL, NULL, &w, &h);

	ldown_dst.w=w*scale;
	ldown_dst.h=h*scale;
	ldown_dst.x=4;
	ldown_dst.y=ctrl_dst.y-ldown_dst.h;
	
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
					p1.cur.vy = -paddle_speed*ms;
					break;
				case SDLK_LCTRL:
					p1.afk_time=0;
					if (p1.human==false)
						p1.human=true;
					p1.cur.vy = paddle_speed*ms;
					break;
				case SDLK_RSHIFT:
					p2.afk_time=0;
					if (p2.human==false)
						p2.human=true;
					p2.cur.vy = -paddle_speed*ms;
					break;
				case SDLK_RCTRL:
					p2.afk_time=0;
					if (p2.human==false)
						p2.human=true;
					p2.cur.vy = paddle_speed*ms;
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
					p1.cur.vy = 0;
					break;
				case SDLK_LCTRL:
					p1.timer=true;
					p1.cur.vy = 0;
					break;
				case SDLK_RSHIFT:
					p2.timer=true;
					p2.cur.vy = 0;
					break;
				case SDLK_RCTRL:
					p2.timer=true;
					p2.cur.vy = 0;
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
		SDL_RenderCopy(Renderer, p1.ava, NULL, &p1.ava_dst);
	else
		SDL_RenderCopy(Renderer, cpu, NULL, &cpu_dst);
	if (p2.human==true)
		SDL_RenderCopy(Renderer, p2.ava, NULL, &p2.ava_dst);
	else
		SDL_RenderCopy(Renderer, cpu, NULL, &cpu_dst2);
	
	SDL_RenderCopy(Renderer, shift, NULL, &shift_dst);
	SDL_RenderCopy(Renderer, shift, NULL, &shift_dst2);
	SDL_RenderCopy(Renderer, ctrl, NULL, &ctrl_dst);
	SDL_RenderCopy(Renderer, ctrl, NULL, &ctrl_dst2);
	
	if (p1.cur.vy<0)
		SDL_RenderCopy(Renderer, lup, NULL, &lup_dst);
	if (p1.cur.vy>0)	
		SDL_RenderCopy(Renderer, ldown, NULL, &ldown_dst);
	if (p2.cur.vy<0)
		SDL_RenderCopy(Renderer, rup, NULL, &rup_dst);
	if (p2.cur.vy>0)	
		SDL_RenderCopy(Renderer, rdown, NULL, &rdown_dst);
}

void PlayingBye(void)
{

	SDL_DestroyTexture(cpu);
	SDL_DestroyTexture(shift);
	SDL_DestroyTexture(ctrl);
	SDL_DestroyTexture(rup);
	SDL_DestroyTexture(rdown);
	SDL_DestroyTexture(lup);
	SDL_DestroyTexture(ldown);
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

	char 	  string[]={"PAUSED"};
	SDL_Color color={BLUE}; 
	temp_surface = TTF_RenderText_Solid(font, string, color);
	paused  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(paused, NULL, NULL, &paused_dst.w, &paused_dst.h);
	paused_dst.x=(ww/2)-(paused_dst.w/2);
	paused_dst.y=(wh/2)-(paused_dst.h/2);
	
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
	
SDL_RenderCopy(Renderer, paused, NULL, &paused_dst);
	
}

void PausedBye(void)
{
	SDL_DestroyTexture(paused);
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
	SDL_Color 	color={BLUE}; 
	
	char 		string[]={"GAME OVER"};
	
	temp_surface = TTF_RenderText_Solid(font, string, color);
	game_over  = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(game_over, NULL, NULL, &game_over_dst.w, &game_over_dst.h);
	game_over_dst.x=(ww/2)-(game_over_dst.w/2);
	game_over_dst.y=game_over_dst.h*2;
	
	char 		string1[]={"Player 1 won"};
	
	temp_surface = TTF_RenderText_Solid(font, string1, color);
	p1.go = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(p1.go, NULL, NULL, &p1.go_dst.w, &p1.go_dst.h);
	p1.go_dst.x=(ww/2)-(p1.go_dst.w/2)-14;
	p1.go_dst.y=(wh/2)+(p1.go_dst.h/2);
	
	char 		string2[]={"Player 2 won"};
	
	temp_surface = TTF_RenderText_Solid(font, string2, color);
	p2.go = SDL_CreateTextureFromSurface(Renderer,temp_surface );
	SDL_QueryTexture(p2.go, NULL, NULL, &p2.go_dst.w, &p2.go_dst.h);
	p2.go_dst.x=(ww/2)-(p2.go_dst.w/2)-14;
	p2.go_dst.y=(wh/2)+(p2.go_dst.h/2);
	
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
	SDL_RenderCopy(Renderer, game_over, NULL, &game_over_dst);
	if(p1.score==END_SCORE)
		SDL_RenderCopy(Renderer, p1.go, NULL, &p1.go_dst);
	else
		SDL_RenderCopy(Renderer, p2.go, NULL, &p2.go_dst);
}

void Game_overBye(void)
{
	SDL_DestroyTexture(game_over);
}

//END 	GAME_OVER

//END 	GAME STATE FUNCTIONS

//BEGIN HELPER
void assets_in(void)
{

	hit  = Mix_LoadWAV("./assets/hit.wav");
	Mix_VolumeChunk(hit, 35);
	wall = Mix_LoadWAV("./assets/wall.wav");
	Mix_VolumeChunk(wall, 35);
	
	font = TTF_OpenFont("./assets/8-bit.ttf", 28);
	
	//BEGIN LOGO
	temp_surface = IMG_Load("./assets/gfx/logo.png");
	logo = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_SetTextureAlphaMod(logo,100);
	SDL_QueryTexture(logo, NULL, NULL, &logo_dst.w, &logo_dst.h);
	logo_dst.x=(ww/2)-(logo_dst.w/2);
	logo_dst.y=(wh/2)-(logo_dst.h/2);
	//END 	LOGO
	
	IdleReady();
	PlayingReady();
	PausedReady();
	Game_overReady();
}

void assets_out(void)
{
	SDL_FreeSurface(temp_surface);
	SDL_DestroyTexture(logo);
	Mix_FreeChunk(hit);
	Mix_FreeChunk(wall);
	TTF_CloseFont(font);
	IdleBye();
	PlayingBye();
	PausedBye();
	Game_overBye();
}
//END 	HELPER
//END   FUNCTIONS
