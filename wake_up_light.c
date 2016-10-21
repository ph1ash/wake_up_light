#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define True 1
#define False 0

//Wake up time info
static uint8_t wake_up_hour = 5;
static uint8_t wake_up_minute = 30;
static uint16_t wake_up_duration = 30;

typedef struct Light
{
    int state;
    void (*set_pwm)(uint16_t);
    void (*on)(void);
    void (*off)(void);
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

void set_light_pwm(uint16_t pwm)
{
    // Set PWM
    printf("Setting PWM to %d\n", pwm);
    pwmWrite(1, pwm);
}

void turn_on_light(void)
{
    // Turn on the light
    printf("Turning on light");
    pwmWrite(1, 1024);
}

void turn_off_light(void)
{
    // Turn off the light
    printf("Turning off light");
    pwmWrite(1, 0);
}

Light_t *init(Light_t *light)
{
    if (wiringPiSetup() == -1)
        exit(1);

    Light_t *new_light = malloc(sizeof(Light_t));
    new_light->set_pwm = &set_light_pwm;
    new_light->on = &turn_on_light;
    new_light->off = &turn_off_light;
    new_light->state = False;    
    new_light->set_pwm(0);
    pinMode(1, PWM_OUTPUT);

    return new_light;
}

int main(void)
{
    Light_t *m_light = malloc(sizeof(Light_t));
    m_light = init(m_light);

	struct tm *curr_time;
    
    curr_time = get_time_of_day();
    
    //uint8_t start_hour = curr_time->tm_hour;
    //uint8_t start_min = curr_time->tm_min;

    //Loop every minute to see if the light should be turned on
    while(1)
    {
        curr_time = get_time_of_day();

        uint8_t hour = curr_time->tm_hour;
        uint8_t min = curr_time->tm_min;

     	printf("Current time = %d:%d\n", hour, min);

        if ((hour == wake_up_hour) && (min >= wake_up_minute))
        {
            printf("Time to wake up!\n");
            uint8_t idx;
            for (idx = 1; idx <= wake_up_duration; idx++)
            {
                static uint16_t pwm = 0;
                pwm = ((1024/wake_up_duration)*(idx));
                m_light->set_pwm(pwm); 
                sleep(60); //Sleep for a minute
            }
            printf("Keeping light on for 15 mins");
            fflush(stdout);
            sleep(900); // Keep lights on for 15 mins
            m_light->off();
        }
        else
        {
            printf("zzz...\n");
            fflush(stdout);
            sleep(60); //Sleep for a minute
        }
    }
}
