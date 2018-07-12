#include <mbed.h>
#include <Joystick.h>
 
//                  y     x     button
Joystick joystick(A0,A2,D2);
 
int main() {
    
    joystick.init();
    
    while(1) {
    
        Vector2D coord = joystick.get_coord();
        printf("Coord = %f,%f\n",coord.x,coord.y);
        
        Vector2D mapped_coord = joystick.get_mapped_coord(); 
        printf("Mapped coord = %f,%f\n",mapped_coord.x,mapped_coord.y); 
        
        float mag = joystick.get_mag();
        float angle = joystick.get_angle();
        printf("Mag = %f Angle = %f\n",mag,angle);
        
        Direction d = joystick.get_direction();
        printf("Direction = %i\n",d);
        
        if (joystick.button_pressed() ) {
            printf("Button Pressed\n");  
        }
          
        wait(.1);
    }
    
    
}