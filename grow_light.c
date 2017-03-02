#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define ON 1
#define OFF 0
//#define DEBUG

//Wake up time info
static uint8_t wake_up_hour = 6;
static uint8_t shutdown_hour = 21;

typedef struct Light
{
    char* light_gpio;
    char* fan_gpio;
    void (*turn_light)(int, char*);
    void (*turn_fan)(int, char*);
} Light_t;

struct tm *get_time_of_day(void)
{
	time_t now;
	struct tm *tm;

	now = time(0);
	if ((tm = localtime (&now)) == NULL)
	{
		printf ("Error extracting time stuff\n");
		return NULL;
	}
	
    return tm;
}


// Generic handler for turning on or off a device
// Takes in the state and address of the GPIO
void turn_device(int state, char* gpio_number)
{
    char value_path[128];
    strcpy(value_path, "/sys/class/gpio/");
    strcat(value_path, gpio_number);
    strcat(value_path, "/value");  

    FILE *f = fopen(value_path, "w");
    if (f == NULL)
    {
        printf("Error accessing GPIO\n");
        exit(1);
    }
    else
    {
        if (state)
        {
            // Relay module has polarity inverted (off -> 1, on -> 0)
            fprintf(f, "0");
        }
        else
        {
            fprintf(f, "1");
        }
    }
    fclose(f);
}

void set_gpio_direction(char* gpio_number)
{
    FILE *f = fopen("/sys/class/gpio/export", "w");

    if (f == NULL)
    {
        printf("Error exporting\n");
        exit(1);
    }
    else
    {
        fprintf(f, gpio_number);
    }
    fclose(f);

    char direction_path[128];
    strcpy(direction_path, "/sys/class/gpio/");
    strcat(direction_path, gpio_number);
    strcat(direction_path, "/direction");

    f = fopen(direction_path, "w");
    if (f == NULL)
    {
        printf("Error opening GPIO direction\n");
        exit(1);
    }
    else
    {
        fprintf(f, "out");
    }
    fclose(f);

}

Light_t *init(Light_t *light)
{
    Light_t *new_light = malloc(sizeof(Light_t));
    new_light->turn_light = &turn_device;
    new_light->turn_fan = &turn_device;
    new_light->light_gpio = "gpio1019";
    new_light->fan_gpio = "gpio1020";

    set_gpio_direction(new_light->light_gpio);
    set_gpio_direction(new_light->fan_gpio);

    return new_light;
}

void debug_loop(Light_t *light)
{
    printf("Turning things on");
    fflush(stdout);
    light->turn_light(ON, light->light_gpio);
    light->turn_fan(ON, light->fan_gpio);
    sleep(10);
    printf("Turning things off");
    fflush(stdout);
    light->turn_light(OFF, light->light_gpio);
    light->turn_fan(OFF, light->fan_gpio);
    sleep(10);
}

int main(void)
{
    Light_t *m_light = malloc(sizeof(Light_t));
    m_light = init(m_light);

	struct tm *curr_time;
    
    curr_time = get_time_of_day();
   
#ifdef DEBUG
    while(1)
    {
        debug_loop(m_light);
    }
#endif

    //Loop every minute to see if the light should be turned on
    while(1)
    {
        curr_time = get_time_of_day();

        uint8_t hour = curr_time->tm_hour;

     	printf("Current time = %d\n", hour);
        fflush(stdout);

        if ((hour >= wake_up_hour) && (hour <= shutdown_hour))
        {
            m_light->turn_light(ON, m_light->light_gpio);
            m_light->turn_fan(OFF, m_light->fan_gpio);
        }
        else
        {
            m_light->turn_light(OFF, m_light->light_gpio);
            m_light->turn_fan(ON, m_light->fan_gpio);
        }
        sleep(60);
    }
}
