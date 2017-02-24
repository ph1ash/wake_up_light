#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define True 1
#define False 0

//Wake up time info
static uint8_t wake_up_hour = 6;
static uint8_t shutdown_hour = 21;

typedef struct Light
{
    int state;
    char* gpio_addr;
    void (*on)(char*);
    void (*off)(char*);
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

void turn_on_light(char * gpio_addr)
{
    // Turn on the light
    printf("Turning on light\n");
    fflush(stdout);

    FILE *f = fopen(gpio_addr, "w");
    if (f == NULL)
    {
        printf("Error accessing value\n");
        exit(1);
    }
    else
    {
        // Relay module has polarity inverted (off -> 1, on -> 0)
        fprintf(f, "0");
    }
    fclose(f);
}

void turn_off_light(char * gpio_addr)
{
    // Turn off the light
    printf("Turning off light\n");
    fflush(stdout);

    FILE *f = fopen(gpio_addr, "w");
    if (f == NULL)
    {
        printf("Error accessing value\n");
        exit(1);
    }
    else
    {
        // Relay module has polarity inverted (off -> 1, on -> 0)
        fprintf(f, "1");
    }
    fclose(f);
}

Light_t *init(Light_t *light)
{

    FILE *f = fopen("/sys/class/gpio/export", "w");

    if (f == NULL)
    {
        printf("Error exporting\n");
        exit(1);
    }
    else
    {
        fprintf(f, "1019");
    }
    fclose(f);

    f = fopen("/sys/class/gpio/gpio1019/direction", "w");
    if (f == NULL)
    {
        printf("Error getting direction\n");
        exit(1);
    }
    else
    {
        fprintf(f, "out");
    }
    fclose(f);

    Light_t *new_light = malloc(sizeof(Light_t));
    new_light->on = &turn_on_light;
    new_light->off = &turn_off_light;
    new_light->state = False;    
    new_light->gpio_addr = "/sys/class/gpio/gpio1019/value";
    return new_light;
}

void debug_loop(Light_t *light)
{
    light->on(light->gpio_addr);
    sleep(10);
    light->off(light->gpio_addr);
    sleep(10);
}

int main(void)
{
    Light_t *m_light = malloc(sizeof(Light_t));
    m_light = init(m_light);

	struct tm *curr_time;
    
    curr_time = get_time_of_day();
   
/*    while(1)
    {
        debug_loop(m_light);
    }*/

    //Loop every minute to see if the light should be turned on
    while(1)
    {
        curr_time = get_time_of_day();

        uint8_t hour = curr_time->tm_hour;

     	printf("Current time = %d\n", hour);
        fflush(stdout);

        if ((hour >= wake_up_hour) && (hour <= shutdown_hour))
        {
            m_light->on(m_light->gpio_addr);
        }
        else
        {
            m_light->off(m_light->gpio_addr);
        }
        sleep(60);
    }
}
