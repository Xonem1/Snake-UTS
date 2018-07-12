// Librerias
#include <mbed.h>
#include "main.h"
#include <Nokia5110.h>
#include <Joystick.h>
#include <Speaker.h>

// Salidas a pins
Nokia5110 display(D8,D9,D12,D11,D13);
Joystick joystick(A0,A2,D2);
Speaker mySpeaker(D6);

// Ticker
Ticker move;

// Variables de control
u_int8_t score = 0;
float fps=0.1;
enum state
{       
    start, stop, run, pause
};
enum directions{ up=5, down=1, left=7, right=3, null=0};
directions MovDir;
state game_state;
int map[MAX_WIDTH][MAX_HEIGHT]; //si 0=vacio, 1=fruta, 2=muro
//int fruit_pos[0][0];
//int _pos[0][0];

// Clases
struct objeto
{
    int x;
    int y;
};

struct objeto head;
struct objeto fruit;
struct objeto wall;
objeto* corp = new objeto[score+5];

// Funciones
//Fruta
void SetFruit(){
    fruit.x = rand()%MAX_WIDTH+1;
    fruit.y = rand()%MAX_HEIGHT+1;
    map[fruit.x][fruit.y] = 1;
    if(map[fruit.x][fruit.y] == 2){
        SetFruit();
    }
}
//wall
void SetWall(){
    wall.x = rand()%MAX_WIDTH+1;
    wall.y = rand()%MAX_HEIGHT+1;
    map[wall.x][wall.y] = 1;
    if(map[wall.x][wall.y] == 2){
        SetWall();
    }
}

// Direcciones
void up_Dir(){
    if(MovDir != down){
        MovDir = up;
    }
}

void down_Dir(){
    if(MovDir != up){
        MovDir = down;
    }
}

void left_Dir(){
    if(MovDir != right){
        MovDir = left;
    }
}

void right_Dir(){
    if(MovDir != left){
        MovDir = right;
    }
}

void Push_Touch(){
    wait(0.5);
    if(game_state==run){
        MovDir=null;
        //game_state=pause;
    }else if(game_state==pause){
        game_state=run;
    }else if(game_state == stop){
        for(int i=0;i<=4;i++){
            corp[i].x=(head.x)-(i+1);
            corp[i].y=15;
        }
        MovDir=null;
        game_state=run;
    }
}

// Move the snake
void MoveSnake(){
    if(game_state==run){
        if(MovDir!=null){
            for(int i=(score+4);i>=1;i--){
                corp[i]=corp[i-1];
            }
            corp[0]=head;
        }
        switch(MovDir){
            case up:
                head.y-=1;
                break;
            case down:
                head.y+=1;
                break;
            case left:
                head.x-=1;
                break;
            case right:
                head.x+=1;
                break;

        }
// Crashed
        if(head.x==127 ||head.x==0 ||head.y==0 ||head.y==31 ){
            display.clear_buffer();
            display.print_string("GameOver Perro!",33,5);
            display.print_string("Your score is :",33,15);
            MovDir=null;
            head.x=15;
            head.y=15;
            game_state=stop;
            score=0;
            fps=0.1;
        }else if((head.x==fruit.x)&&(head.y==fruit.y)){
          //Eat the mouse
            score+=1;
            SetFruit();
            fps=fps-0.01;
            return move.attach(&MoveSnake, fps);
        }else{
            display.clear_buffer();
            display.draw_pixel(head.x,head.y,1);
            display.display();
            for(int k=0;k<=(score+4);k++){
                display.draw_pixel(corp[k].x,corp[k].y,1);
            }
            display.display();
            display.draw_pixel(fruit.x,fruit.y,1);
            display.display();
            display.draw_rect(0,0, 83, 47);
            display.display();
        }
// Game Over
        for(int a=0;a<=(score+4);a++){
            if(corp[a].x==head.x && corp[a].y==head.y){
                display.clear_buffer();
                display.print_string("Game Over Perro!",13,5);
                display.print_string("Your score is :",13,15);
                MovDir=null;
                head.x=15;
                head.y=15;
                game_state=stop;
                score=0;
                fps=0.1;
                display.display();
            }
        }
    }
    //Hold the Game
    else if(game_state==pause){
        display.clear_buffer();
        display.print_string("Pause",13,15);
        display.display();
    }
}


int main() {
    joystick.init();
    display.init(0x2C);
    display.clear_buffer();
    int m=0;
    int p=0;
 
    /*
    char* tab_menu[3];
        tab_menu[0]="Snake";
        tab_menu[1]="Snake Hardcore";
        tab_menu[2]="???";
    */

    bool isStarted=false;
 
    // Menu
    while(isStarted==false){
        int j=0;
 
        for(j=10;j>0;j--){
            display.clear_buffer();
            display.print_string(":V Snake!!",3,j);
            mySpeaker.PlayNote(45*j,0.1,0.1);
            //display.print_string("Snake!",13,j);
            wait(.2);
            display.display();
        }
        display.clear_buffer();
        display.print_string("Press JoyStick",0,5);
        display.display();
 
        while(1) {
            //display.print_string(tab_menu[m],0,15);
            //Direction joydir = joystick.get_direction();
            /*int d = joydir;
            if(d == up){
                m=(m-1+3)%3;
                display.clear_buffer();
                display.print_string("Escoje un Juego",0,2);
                display.print_string(tab_menu[m], 0,15);
                display.display();
                wait(0.5);
            }
            else if(d == down){
                m=(m+1+3)%3;
                display.clear_buffer();
                display.print_string("Escoje un Juego",0,2);
                display.print_string(tab_menu[m], 0,15);
                wait(0.5);
                display.display();
            }
            else */
            bool button = joystick.get_direction();
            if(button){
                p=m;
                isStarted=true;
                break;
 
            }
        }
    }
 
    // Snake game_state
    while(isStarted==true && p==0){
        int j=0;
        display.clear_buffer();
 
        for(j=10;j>0;j--){
            display.clear_buffer();
            display.print_string("I",0,j);
            mySpeaker.PlayNote(100*j,0.1,0.1);
            display.print_string("Snake game_state",13,5);
            mySpeaker.PlayNote(40.0/j,0.1,0.5); 
            display.display();
        }
        /*
        display.locate(0,2);
        for(int f=3;f>=0;f--){
            display.locate(0,2);
            display.printf("Snake begin in : %d",f);
            wait(1.0);
            display.cls();
        }*/
 
        //Snake start
        game_state=run;
        display.draw_rect(0,0,83,47);
        display.display();
        MovDir=null;
        MovDir=right;
        head.x=15;
        head.y=15;
        for (int i=0;i<=4;i++){
            corp[i].x=(head.x)-(i+1);
            corp[i].y=15;
        }
        SetFruit();
        move.attach(&MoveSnake, 0.1);
        while (1){
            Direction joydir = joystick.get_direction();
            bool button = joystick.get_direction();
            int d = joydir;
            if(d == up)
                up_Dir();
            else if(d == down)
                down_Dir();
            else if(d == left)
                left_Dir();
            else if (d == right)
                right_Dir();/*
            else if (button){
                Push_Touch();}*/
        }
    }
 
    //Snake Hardcore
    
        
    
}
 
