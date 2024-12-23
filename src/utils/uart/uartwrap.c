#include "uartwrap.h"

/**
 * @brief open file device
 * 
 * @param com device path
 * @return fd
 */
static int UART_Open_Device(char* com)
{
    int fd;
    fd = open(com, O_RDWR | O_NOCTTY);

    if (-1 == fd)
        return -1;

    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
        return -1;
    }

    return fd;
}

/**
 * @brief Set the opt object
 * 
 * @param fd file device.
 * @param baudRate baud rate, trans speed.
 * @param nBits one time send N bits.
 * @param nEvent parity bit.
 * @param nStop stop bit.
 * @return success or fail.
 * @retval 0, success.
 * @retval -1, fail.
 */
static int UARTWRAP_SetOpt(int fd, int baudRate, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;

    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("SetupSerial 1");
        return -1;
    }

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_oflag &= ~OPOST;

    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch (nEvent) //设置校验位
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }

    switch (baudRate) //设置波特率
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 19200:
        cfsetispeed(&newtio, B19200);
        cfsetospeed(&newtio, B19200);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }

    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;

    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VTIME] = 0;
    tcflush(fd, TCIFLUSH);

    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("com set error");
        return -1;
    }

    return 0;
}

int UART_Init(char* dev_name, int baudrate)
{
    int fd = UART_Open_Device(dev_name);
    if (fd < 0)
        return -1;

    int ret = UARTWRAP_SetOpt(fd, baudrate, 8, 'N', 1);
    if (ret != 0)
        return -2;

    return fd;
}

void UART_Exit(int fd)
{
    if (fd >= 0)
        close(fd);
}