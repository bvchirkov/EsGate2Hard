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

// Реализация взята с публикации по ссылке
// http://dekud.blogspot.com/2013/12/com-linux-com-com-linux.html

// File created 26.06.2019

#pragma once

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int F_ID = -1; ///< Иднетификатор файлового дескриптора

/**
 * @brief openPort Открыть COM порт
 * @param COM_name Путь к устройству, например: /dev/ttyS0 или  /dev/ttyUSB0 - для USB
 * @param speed скорость, например: B9600, B57600, B115200
 * @return false - порт не открылся, true - открылся
 */
int		openPort(const char *COM_name, speed_t speed);
/**
 * @brief readData Прочитать данные из COM порта
 * @param buff Буфер для принятых данных
 * @param size Количество запрашиваемых байт
 * @return Return the number read, -1 for errors or 0 for EOF
 */
ssize_t		readData(unsigned char *buff, unsigned int size);
/**
 * @brief sendData Отправить в COM порт данные
 * @param buff Буфер данных для отправки
 * @param len Количество отправляемых байт
 * @return Return the number written, or -1.
 */
ssize_t		sendData(const unsigned char *buff, unsigned int len);
/**
 * @brief closeCom Закрыть COM порт
 */
void		closeCom();


int openPort(const char *COM_name, speed_t speed)
{
    F_ID = open(COM_name, O_RDWR | O_NOCTTY);
    if(F_ID == -1)
    {
		char *errmsg = strerror(errno);
		printf("%s\n", errmsg);
		return 0;
    }

    struct termios options;     /*структура для установки порта*/
    tcgetattr(F_ID, &options);  /*читает пораметры порта*/

    cfsetispeed(&options, speed); /*установка скорости порта*/
    cfsetospeed(&options, speed); /*установка скорости порта*/

    options.c_cc[VTIME]    = 20; /*Время ожидания байта 20*0.1 = 2 секунды */
    options.c_cc[VMIN]     = 0;  /*минимальное число байт для чтения*/

    options.c_cflag &= ~PARENB; /*бит четности не используется*/
    options.c_cflag &= ~CSTOPB; /*1 стоп бит */
    options.c_cflag &= ~CSIZE;  /*Размер байта*/
    options.c_cflag |= CS8;     /*8 бит*/

    options.c_lflag = 0;
    options.c_oflag &= ~OPOST; /*Обязательно отключить постобработку*/

    tcsetattr(F_ID, TCSANOW, &options); /*сохронения параметров порта*/

    return 1;
}

ssize_t readData(unsigned char *buff, unsigned int size)
{
    ssize_t n = read(F_ID, buff, size);
    if(n == -1)
    {
		char *errmsg = strerror(errno);
		printf("%s\n", errmsg);
    }
    return n;
}

ssize_t sendData(const unsigned char* buff, unsigned int len)
{
    ssize_t n = write(F_ID, buff, len);
    if(n == -1)
    {
		char *errmsg = strerror(errno);
		printf("%s\n", errmsg);
    }
    return n;
}

void closeCom(void)
{
    close(F_ID);
    F_ID = -1;
}

