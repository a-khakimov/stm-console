#include "console.h"
#include "stm32f7xx_hal_usart.h"


extern UART_HandleTypeDef huart3;

uint8_t receive_finish = 0;
void 	set_receive_finish( const uint8_t state) { receive_finish = state;}
uint8_t get_receive_finish( ) {	return receive_finish; }

/* Функция для посылки данных по uart */
void console_send(char *str, size_t size) {
	HAL_UART_Transmit(&huart3, str, size, 100);
}


/**************************************************************/
/* help - вывод списка всех команд
 */

const char *help_text = "Command list:\n"
		"reset - reset controller\n"
		"ticks - get work time in ms\n"
		"led - stupid led switcher [-show][-b][-r]"
		"[set][reset]\n"
		"logout - end of session\n";

void dev_help(uint8_t* rx_buf, size_t cur_indx) {
	console_send(rx_buf, strlen(rx_buf));
	console_send(help_text, strlen(help_text));
}

/**************************************************************/
/* Сброс контроллера
 */
uint8_t reset_state = 1;	// Для текущего состояния устройства
void 	set_reset_state( const uint8_t state) { reset_state = state;}
uint8_t get_reset_state( 					) {	return reset_state; }

void dev_reset(uint8_t* rx_buf, size_t cur_indx) {
	HAL_UART_Transmit(&huart3, rx_buf, cur_indx, 1000);
	set_reset_state(0);
}

/**************************************************************/
/* Вывод тиков (время работы контроллера)
 */
void dev_tick(uint8_t* rx_buf, size_t cur_indx) {
	char str[30];
	sprintf(str, "%sTick: %" PRIu32 " ms \n", rx_buf, HAL_GetTick());
	console_send(str, strlen(str));
}

/**************************************************************/
/* Реализация переключения светодиодов 
 * 
 */

#define LED_RED(state)	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, state)
#define LED_BLUE(state)	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, state)


void dev_led(uint8_t* rx_buf, size_t cur_indx) {

	char **argv = NULL, *p = NULL;
    int argc = 0;
    // раскидываем слова по отдельным массивам (в массив указателей argv)
    for (p = strtok(rx_buf, " "); p != NULL; p = strtok(NULL, " ")) {
        argv = (char**)realloc(argv, sizeof(char*) * (argc + 1));
        argv[argc] = strdup(p);
        ++argc;
    }
    
    int opt = 0;
    while((opt = getopt(argc, argv, ":s:r:b:s:r")) != -1) {
        switch(opt) {
        case 's':
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14))
				console_send("\nred: set", 9);
			else 
				console_send("\nred: reset", 11);
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7))
				console_send("\nblue: set\n", 11);
			else
				console_send("\nblue: reset\n", 13);			
			return;

        case 'r':
            console_send("-red\n", 5);
            if (optarg[0] == 's')
            	LED_RED(GPIO_PIN_SET);
            else if (optarg[0] == 'r')
            	LED_RED(GPIO_PIN_RESET);
            break;
        case 'b':
            console_send("-blue\n", 6);
            if (optarg[0] == 's')
            	LED_BLUE(GPIO_PIN_SET);
            else if (optarg[0] == 'r')
            	LED_BLUE(GPIO_PIN_RESET);
            break;
        }
    }

    // чистим за собой мусор
    for (int i = 0; i < argc; ++i) {
        if (argv[i] != NULL) {
            free(argv[i]);
            argv[i] = NULL;
        }
    }

    free(argv);
    argv = NULL;
}


/**************************************************************/
/* Примитивная реализация входа в систему управления.
 * Без возможности изменения логина и пароля.
 * По умолчанию 
 		логин: user
 		пароль: 1234
 */
const char *user = "user";
const char *pass = "1234";
uint8_t login_state = 0;
void 	set_login_state( const uint8_t state) { login_state = state;}
uint8_t get_login_state() {	return login_state; }

void dev_login(uint8_t* rx_buf, size_t cur_indx) {
	if(get_login_state()) {
		set_login_state(0);
		console_send("Enter [username password]\n", 26);
		return;
	}
	char str[20];
	console_send("Enter [username password]\n", 26);
	if(!strncmp(user, rx_buf, strlen(user))) {
		char* tmp_pass = rx_buf + strlen(user) + 1;
		if((strlen(tmp_pass)-1) == strlen(pass)) {
			if(!strncmp(tmp_pass, pass, strlen(pass))) {
				set_login_state(1);
				console_send("Welcome to the console\n", 23);
				console_send("Enter [help] for get command list\n", 34);
			}
		}		
	}
}


/**************************************************************/
/* Домашнее задание. Список команд:
 * help - вывод доступных команд с описанием
 * reset - рестарт платы (для рестарта нужно использовать Watchdog)
 * ticks - вывод системных тиков
 * led - переключить (toggle) светодиод 
 * xxx - придумать свою команду 
*/

struct command {
  const char *name;
  void (*handle)();
};

struct command commands[] = {
	{ "help", 	dev_help 	}, 
	{ "reset", 	dev_reset 	},
	{ "ticks", 	dev_tick 	},
	{ "led", 	dev_led 	},
	// .. следующие команды в будущем добавлять до этой строки
	{ "logout", dev_login 	},
};

/**************************************************************/
/* Обработчик принятых с UART данных
 * Выполняет сравнение и при совпадении вызов соответствующей функции
 */
void console_handler(uint8_t* rx_buf, size_t cur_indx) {
	optind = 1;
	if(get_login_state()) {
		console_send("\nuser$ ", 7);
	    for(int i = 0; i < sizeof(commands)/sizeof(struct command); ++i) {
	    	if( !strncmp( commands[i].name, rx_buf, strlen(commands[i].name) - 1 )) {
	    		commands[i].handle(rx_buf, cur_indx);
	    		break;
	    	}
	    }
	} else {
		commands[sizeof(commands)/sizeof(struct command) - 1].handle(rx_buf, cur_indx);
	}
}



