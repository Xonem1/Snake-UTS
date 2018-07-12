// Librerias
#include <mbed.h>
#include <Nokia5110.h>
#include <Joystick.h>
#include <Speaker.h>

// Salidas a pins
Nokia5110 display(D8,D9,D12,D11,D13);
Joystick joystick(A0,A2,D2);

static int X_buffer = 42;
static int Y_buffer = 24;

void move_pixel(int x,int y);


int main() {
    // CONFIG
    joystick.init();
    display.init(0x2C);
    display.clear_buffer();
    //display.draw_pixel(X_buffer,Y_buffer,1);
    display.draw_rect(X_buffer,Y_buffer,X_buffer+2,Y_buffer+2);
    display.display();

    while(1)
    {    
        Direction d = joystick.get_direction();
        //printf("Direction = %i  -",d);
        printf("  X:%d -- Y:%d \n",X_buffer,Y_buffer);
        if (joystick.button_pressed() ) {
            printf("Button Pressed\n");  
        }
        
        switch (d)
        {
            case 1:
                //printf("abajo");
                move_pixel(0,1);
                break;

            case 3:
                //printf("derecha");
                move_pixel(1,0);
                break;

            case 5:
                //printf("arriba");
                move_pixel(0,-1);
                break;
            
            case 7:
                //printf("izquierda");
                move_pixel(-1,0);
                break;

            default:
                break;
        }
        wait_ms(50);
        speaker=0.0;
    }
    
}
void move_pixel(int x,int y){
    speaker.period(1.0/440.0);
    speaker=0.5;
    display.clear_buffer();
    X_buffer +=x;
    Y_buffer +=y;
    
    if(X_buffer<0){X_buffer=83;}
    if(Y_buffer<0){Y_buffer=47;}
    if(X_buffer==84){X_buffer=0;}
    if(Y_buffer==48){Y_buffer=0;}

    //display.draw_pixel(X_buffer,Y_buffer,1);
    display.draw_rect(X_buffer,Y_buffer,X_buffer+2,Y_buffer+2);
    display.display();
}