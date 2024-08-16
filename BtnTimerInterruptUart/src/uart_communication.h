//	EmSyPro9000
// 	Embedded system programming 2022

// UART communication library header file

void initialize_uart(void);
//void uart_send(char);
void uart_send_string(char []);
char uart_receive();
void uart_send_config(int, int, int);
