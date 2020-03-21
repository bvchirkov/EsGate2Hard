/*Copyright (c) 2019 Boris Chirkov

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the 
"Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, 
distribute, sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so, subject to 
the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// File created 26.06.2019
//wcalc -b "(256 | 3) & 0xff"

#include "EsCOMTools.h"
#include "EsPrintBits.h"
#include "EsDelay.h"

#define STATUS		( 0x00 )
#define SET		( 0x01 )

#define IDLE		( 0x00 )
#define CMD_OFF		( 0x00 )

#define CMD_LEFT	( 0x01 )	
#define CMD_RIGHT	( 0x02 )	

#define CMD_STOP	( 0x01 )
#define CMD_GO		( 0x02 )

#define INTERVAL	( 1000 )	 //ms

#define NUM_OF_ARW 	( 2 )
#define NUM_OF_LHT 	( 3 )
#define NUM_OF_BTN 	( 3 )

const unsigned char arws[NUM_OF_ARW] = {10, 12};
const unsigned char lhts[NUM_OF_LHT] = {20, 21, 22};
const unsigned char btns[NUM_OF_BTN] = {30, 31, 32};

const unsigned char BROADCAST_ADDR = 128;	

typedef struct
{
    unsigned char addr;
    unsigned char cmd;
    unsigned char data;
    char*   	  str;
} PKG;

struct Btn
{
    unsigned char addr;
    unsigned char state;
    unsigned char room;
};

void read_btn_status(struct Btn*);
unsigned int push_pkg(PKG *);
unsigned int arw_test_on_off(void);
unsigned int lht_test_on_off(void);
unsigned int work_loop(void);
unsigned int userCmds(void);

unsigned char _check_00(unsigned char);
unsigned char _check_01(unsigned char);
unsigned char _check_10(unsigned char);

int main(/*int argc, char** argv*/)
{
    // ======= MANUAL TEST ======
    //userCmds();
    
    // ======= AUTOTESTS ========
    //arw_test_on_off();
    //lht_test_on_off();

    // ======= WORK EVERYDAY ========
    work_loop();
   
    return 0;
}

unsigned int work_loop(void)
{
    int res = openPort("/dev/ttyUSB0", B9600);
    if(!res)
    {
        printf("Невозможно открыть COM порт\n");
        return 0;
    }
    
    printf("--------Старт системы------------------\n");
    unsigned char mainloop = 1;
    struct Btn btn_s[3] = {
			    {addr:btns[0], state:0, room:0}, 
			    {addr:btns[1], state:0, room:1},
			    {addr:btns[2], state:0, room:2}
			  };
    PKG _arw_left  = {addr: 0, cmd: SET, data: CMD_LEFT, str: "CMD_LEFT"};
    PKG _arw_right = {addr: 0, cmd: SET, data: CMD_RIGHT, str: "CMD_RIGHT"};
    PKG _lht_go    = {addr: 0, cmd: SET, data: CMD_GO, str: "CMD_GO"};
    PKG _lht_stop  = {addr: 0, cmd: SET, data: CMD_STOP, str: "CMD_STOP"};
    PKG _off       = {addr: 0, cmd: SET, data: CMD_OFF, str: "CMD_OFF"};

        
    while(mainloop)
    {
	printf("* Опрос ручных пожарных извещателей:\n");
	unsigned char flag_on_fire = 0;
	for (size_t i = 0; i < NUM_OF_BTN; i++)
	{
	    read_btn_status(&btn_s[i]);
	    printf("\t#%u ............. %u", btn_s[i].addr, btn_s[i].state);
	    if (btn_s[i].state == 1) 
	    {
		flag_on_fire = 1;
		printf(" *\n");
	    }
	    else printf("\n");
	}
	
	if (flag_on_fire)
	{
	    printf("* Запуск системы управления эвакуацией \n");
	    printf("* Сработал извещатель в помещении ...... ");
	    
	    if (btn_s[0].state == 1) //30
	    {
		printf("#%u\n", btn_s[0].room);
		_arw_right.addr = arws[0]; push_pkg(&_arw_right);
		_arw_right.addr = arws[1]; push_pkg(&_arw_right);
		
		_lht_stop.addr = lhts[0]; push_pkg(&_lht_stop);
		_lht_go.addr = lhts[1]; push_pkg(&_lht_go);
		_lht_stop.addr = lhts[2]; push_pkg(&_lht_stop);
		btn_s[0].state = 0;
	    } else if (btn_s[1].state == 1) //31
	    {
		printf("#%u\n", btn_s[1].room);
		_arw_left.addr = arws[0]; push_pkg(&_arw_left);
		_arw_right.addr = arws[1]; push_pkg(&_arw_right);
		
		_lht_go.addr = lhts[0]; push_pkg(&_lht_go);
		_lht_go.addr = lhts[1]; push_pkg(&_lht_go);
		_lht_stop.addr = lhts[2]; push_pkg(&_lht_stop);
		btn_s[1].state = 0;
	    } else if (btn_s[2].state == 1) //32
	    {
		printf("#%u\n", btn_s[2].room);
		_arw_left.addr = arws[0]; push_pkg(&_arw_left);
		_arw_right.addr = arws[1]; push_pkg(&_arw_right);
		
		_lht_go.addr = lhts[0]; push_pkg(&_lht_go);
		_lht_go.addr = lhts[1]; push_pkg(&_lht_go);
		_lht_stop.addr = lhts[2]; push_pkg(&_lht_stop);
		btn_s[2].state = 0;
	    }
	    _delay_ms(30000);
	    
	    _off.addr = 128; push_pkg(&_off);
	    flag_on_fire = 0;
	}
	
	_delay_ms(2000);
	printf("--------------------------------------------------\n");
    }
    printf("--------Останов системы------------------\n");
    closeCom();
    
    return 1;
}

void print_pkg(PKG* aPkg, char* aStr)
{
    SHOW(unsigned char, aPkg->addr); 
    SHOW(unsigned char, aPkg->cmd); 
    SHOW(unsigned char, aPkg->data);
    printf("%s\n", aStr);
}

void read_btn_status(struct Btn* aBtn)
{
    PKG _status = {addr: aBtn->addr, cmd: STATUS, data: IDLE, str: "STATUS"};
    printf("\t- Проверка статуса\n");
    print_pkg(&_status, _status.str);
    push_pkg(&_status);
    
    char * str = "ТРЕВОГА";
    if ((_status.data & (1 << 4)) != 0) 
    {
	aBtn->state = 1;
    } else str = "ВЫКЛ";
    print_pkg(&_status, str);
}

unsigned int arw_test_on_off()
{
    int res = openPort("/dev/ttyUSB0", B9600);
    if(!res)
    {
        printf("Невозможно открыть COM порт\n");
        return 0;
    }
    
    printf("--------Тестирование стрелок----Start------------------\n");
    
    PKG _status    = {addr: 0, cmd: STATUS, data: IDLE, str: "STATUS"};
    PKG _arw_left  = {addr: 0, cmd: SET, data: CMD_LEFT, str: "CMD_LEFT"};
    PKG _arw_right = {addr: 0, cmd: SET, data: CMD_RIGHT, str: "CMD_RIGHT"};
    PKG _arw_off   = {addr: 0, cmd: SET, data: CMD_OFF, str: "CMD_OFF"};
    
    for (size_t i = 0; i < NUM_OF_ARW; i++)
    {
	printf("* Тестируется световой указатель ............... #%u\n", arws[i]);
	
	{
	    printf("\t- Проверка статуса\n");
	    _status.addr = arws[i];
	    print_pkg(&_status, _status.str);
	    push_pkg(&_status);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_status.data)) 
	    {
		if (_check_01(_status.data)) str = "ЛЕВО";
		else if (_check_10(_status.data)) str = "ПРАВО";
	    } else str = "ВЫКЛ";
	    print_pkg(&_status, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Включение влево\n");
	    _arw_left.addr = arws[i];
	    print_pkg(&_arw_left, _arw_left.str);
	    push_pkg(&_arw_left);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_01(_arw_left.data)) str = "ПРОВАЛ";
	    print_pkg(&_arw_left, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Проверка статуса\n");
	    _status.addr = arws[i];
	    print_pkg(&_status, _status.str);
	    push_pkg(&_status);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_status.data)) 
	    {
		if (_check_01(_status.data)) str = "ЛЕВО";
		else if (_check_10(_status.data)) str = "ПРАВО";
		else str = "ПРОВАЛ";
	    } else str = "ВЫКЛ";
	    print_pkg(&_status, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Включение вправо\n");
	    _arw_right.addr = arws[i];
	    print_pkg(&_arw_right, _arw_right.str);
	    push_pkg(&_arw_right);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_10(_arw_right.data)) str = "ПРОВАЛ";
	    print_pkg(&_arw_right, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Проверка статуса\n");
	    _status.addr = arws[i];
	    print_pkg(&_status, _status.str);
	    push_pkg(&_status);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_status.data)) 
	    {
		if (_check_01(_status.data)) str = "ЛЕВО";
		else if (_check_10(_status.data)) str = "ПРАВО";
		else str = "ПРОВАЛ";
	    } else str = "ВЫКЛ";
	    print_pkg(&_status, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Отключение\n");
	    _arw_off.addr = arws[i];
	    print_pkg(&_arw_off, _arw_off.str);
	    push_pkg(&_arw_off);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_arw_off.data)) str = "ПРОВАЛ";
	    print_pkg(&_arw_off, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Проверка статуса\n");
	    _status.addr = arws[i];
	    print_pkg(&_status, _status.str);
	    push_pkg(&_status);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_status.data)) 
	    {
		if (_check_01(_status.data)) str = "ЛЕВО";
		else if (_check_10(_status.data)) str = "ПРАВО";
		else str = "ПРОВАЛ";
	    } else str = "ВЫКЛ";
	    print_pkg(&_status, str);
	    
	    _delay_ms(INTERVAL);
	}

    }
    printf("--------Тестирование стрелок----Stop-------------------\n");
    closeCom();
    return 1;
}

unsigned int lht_test_on_off()
{
    int res = openPort("/dev/ttyUSB0", B9600);
    if(!res)
    {
        printf("Невозможно открыть COM порт\n");
        return 0;
    }
    
    printf("--------Тестирование светофоров----Start---------------\n");
    
    PKG _status   = {addr: 0, cmd: STATUS, data: IDLE, str: "STATUS"};
    PKG _lht_go   = {addr: 0, cmd: SET, data: CMD_GO, str: "CMD_GO"};
    PKG _lht_stop = {addr: 0, cmd: SET, data: CMD_STOP, str: "CMD_STOP"};
    PKG _lht_off  = {addr: 0, cmd: SET, data: CMD_OFF, str: "CMD_OFF"};
    
    for (size_t i = 0; i < NUM_OF_LHT; i++)
    {
	printf("* Тестируется световой указатель ............... #%u\n", arws[i]);
	
	{
	    printf("\t- Проверка статуса\n");
	    _status.addr = lhts[i];
	    print_pkg(&_status, _status.str);
	    push_pkg(&_status);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_status.data)) 
	    {
		if (_check_01(_status.data)) str = "СТОП";
		else if (_check_10(_status.data)) str = "ИДТИ";
	    } else str = "ВЫКЛ";
	    print_pkg(&_status, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Включение СТОП\n");
	    _lht_stop.addr = lhts[i];
	    print_pkg(&_lht_stop, _lht_stop.str);
	    push_pkg(&_lht_stop);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_01(_lht_stop.data)) str = "ПРОВАЛ";
	    print_pkg(&_lht_stop, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Проверка статуса\n");
	    _status.addr = lhts[i];
	    print_pkg(&_status, _status.str);
	    push_pkg(&_status);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_status.data)) 
	    {
		if (_check_01(_status.data)) str = "СТОП";
		else if (_check_10(_status.data)) str = "ИДТИ";
		else str = "ПРОВАЛ";
	    } else str = "ВЫКЛ";
	    print_pkg(&_status, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Включение ИДТИ\n");
	    _lht_go.addr = lhts[i];
	    print_pkg(&_lht_go, _lht_go.str);
	    push_pkg(&_lht_go);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_10(_lht_go.data)) str = "ПРОВАЛ";
	    print_pkg(&_lht_go, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Проверка статуса\n");
	    _status.addr = lhts[i];
	    print_pkg(&_status, _status.str);
	    push_pkg(&_status);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_status.data)) 
	    {
		if (_check_01(_status.data)) str = "СТОП";
		else if (_check_10(_status.data)) str = "ИДТИ";
		else str = "ПРОВАЛ";
	    } else str = "ВЫКЛ";
	    print_pkg(&_status, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Отключение\n");
	    _lht_off.addr = lhts[i];
	    print_pkg(&_lht_off, _lht_off.str);
	    push_pkg(&_lht_off);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_lht_off.data)) str = "ПРОВАЛ";
	    print_pkg(&_lht_off, str);
	    
	    _delay_ms(INTERVAL);
	}
	
	{
	    printf("\t- Проверка статуса\n");
	    _status.addr = lhts[i];
	    print_pkg(&_status, _status.str);
	    push_pkg(&_status);
	    
	    char * str = "УСПЕШНО";
	    if (!_check_00(_status.data)) 
	    {
		if (_check_01(_status.data)) str = "СТОП";
		else if (_check_10(_status.data)) str = "ИДТИ";
		else str = "ПРОВАЛ";
	    } else str = "ВЫКЛ";
	    print_pkg(&_status, str);
	    
	    _delay_ms(INTERVAL);
	}

    }
    printf("--------Тестирование светофоров----Stop----------------\n");
    closeCom();
    return 1;
}

unsigned int push_pkg(PKG *aPkg)
{
    sendData(&aPkg->addr, 1);
    sendData(&aPkg->cmd,  1);
    sendData(&aPkg->data, 1);

    unsigned char rbuff[3];
    for (unsigned int i = 0; i < 3; i++)
    {
	ssize_t s = readData(&rbuff[i], 1);
	if (s < 1)
	{
	    printf("Нет ответа\n");
	    return 0;
	}
    }
    
    aPkg->addr = rbuff[0];
    aPkg->cmd  = rbuff[1];
    aPkg->data = rbuff[2];
    
    return 1;
}

unsigned int userCmds()
{
    int res = openPort("/dev/ttyUSB0", B9600);
    if(!res)
    {
        printf("Невозможно открыть COM порт\n");
        return 0;
    }
    
    PKG pkg;
    
    size_t count = 1;
    unsigned int interval = 0;
    printf("Введите SLAVE_ADDR: ");
    scanf("%hhu", &pkg.addr);
    printf("Введите CMD: ");
    scanf("%hhu", &pkg.cmd);

    if (pkg.cmd == 1) 
    {
	pkg.str = "SET";
	
	printf("Введите DATA: ");
	scanf("%hhu", &pkg.data);
    } else if (pkg.cmd == 0) 
    {
	pkg.str = "STATUS";
	pkg.data = 0x00;
	
	printf("Введите количество повторений команды: ");
	scanf("%zu", &count);
	
	printf("Введите временной интервал между повторами, мс: ");
	scanf("%d", &interval);
    }
    
    //pkg = {0x01, STATUS, CMD_OFF, "STATUS"};
    //pkg = {0x01, SET, CMD_OFF, "SET, CMD_OFF"};
    
    for (size_t r = 0; r < count; r++)
    {
	sendData(&pkg.addr, 1);
	sendData(&pkg.cmd,  1);
	sendData(&pkg.data, 1);
	
	printf("----------------------------------\n");
	SHOW(unsigned char, pkg.addr); SHOW(unsigned char, pkg.cmd); SHOW(unsigned char, pkg.data);
	printf("%s\n", pkg.str);

	unsigned char rbuff[3];
	size_t i = 0;
	while (i < 3)
	{
	    ssize_t s = readData(&rbuff[i], 1);
	    if (s < 1)
	    {
		printf("Нет ответа\n");
		//return 0;
		i = 10;
		break;
	    }
	    i++;
	}
	
	if (i != 10)
	{
	    SHOW(unsigned char, rbuff[0]); SHOW(unsigned char, rbuff[1]); SHOW(unsigned char, rbuff[2]);
	    printf("\n");
	}
	_delay_ms(interval);
    }

    closeCom();
    return 1;
}

unsigned char _check_00(unsigned char aData)
{
    return (((aData&(1<<0)) == 0) && ((aData&(1<<1)) == 0));
}

unsigned char _check_01(unsigned char aData)
{
    return (((aData&(1<<0)) != 0) && ((aData&(1<<1)) == 0));
}

unsigned char _check_10(unsigned char aData)
{
    return (((aData&(1<<0)) == 0) && ((aData&(1<<1)) != 0));
}